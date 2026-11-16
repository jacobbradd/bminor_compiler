/*
 * code_gen.c
 * Finally, the last stage.
 * We're gonna need the symbol table in order to look up labels.
 * Globals should really be hoisted to the top.
 * Easier to have a separate buffer for this.
 *
 * Calling convention: x86-64 ABI
 *
 * TODO be smarter about r10 and r11: check if they're used and push when necessary
 * Similarly, if scratch is not used then don't push them in function intro...
 *
 * TODO function call is wrong: arguments should be computed before loading
 *      this means function calls have an ershov that is... quite complicated
 *      why? because e.g. f(a, g(2)) then 2 will overwrite a in %rdi
 *
 * I've been manually using a string buffer, but maybe it's just better to output to stdout
 * or a /tmp/ file. But I'm using the string buffer so I don't have to think about the order.
 *
 *
 * TODO find a way of looping through the control flow instead of recursing.
 * (This may not be possible because it's still essentially a tree.)
 *
 *
 * TODO handle when allowed to quickly compute "square" operations (because of side-effects)
 *
 *
 */






#include "dag_simple.h"
#include "ast.h"
#include "sym_table.h"
#include "stdio.h"
#include <stdlib.h>

#include "data_types/stacki.h"
#include "data_types/string_t.h"
#include "data_types/map.h"
#include "cg_helper.h"
#include "code_gen.h"

//////
//Static global stacks which aid codegen
static stacki * cond_label_stack;
static stacki * done_label_stack;

static hash_int * ro_string_holder;

//Constants
static char *param_registers_code_gen[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char fn_intro1[] = "\
    pushq %rbp\n\
    movq %rsp, %rbp\n\
";
static char fn_intro2[] = "\
    pushq %r15\n\
    pushq %r14\n\
    pushq %r13\n\
    pushq %r12\n\
    pushq %rbx\n\
";
static char fn_outro[] = "\
    popq %rbx\n\
    popq %r12\n\
    popq %r13\n\
    popq %r14\n\
    popq %r15\n\
    popq %rbp\n\
    retq\n\
";


//Strings to hold global and ro string code (because it appears before function code)
static string *globl_buffer;
static string *ro_buffer;


void code_add_dag_gen(string *s, struct dag_node *dag, struct SymbolTable* st, int stk_lcls);
string *dag_op_arith_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls);
string * dag_op_bool_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls);
string *dag_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls);
string *stmt_code_gen(struct control_flow_node *cf, struct SymbolTable *st, char* fn_id, int stk_lcls);
string *decl_code_gen(struct control_flow_node *cf, struct SymbolTable *st, int stk_lcls);
void add_global_decl(struct ast_decl *decl);


void code_gen_init_statics(){
    cond_label_stack = stacki_create();
    done_label_stack = stacki_create();
    globl_buffer = string_create_empty();
    ro_buffer = string_create_empty();
    ro_string_holder = hash_int_create_empty();
}

void code_gen_free_statics(){
    stacki_free(done_label_stack);
    stacki_free(cond_label_stack);
    string_free(globl_buffer);
    string_free(ro_buffer);
    hash_int_free(ro_string_holder);
}
//Main function
string *code_gen(struct control_flow_node *cf){
    code_gen_init_statics();
    auto st = create_sym_table();
    sym_table_enter(st); //enter global scope
    auto meat_and_potatos = string_create_empty();
    while(cf){
        auto r = decl_code_gen(cf, st, -1);
        string_append(meat_and_potatos, r);
        string_free(r);
        cf = cf->next;
    }
    sym_table_pop(st); //leave global scope

    /*
     * Build the assembly: globals, strings, and code.
     */
    auto res = string_create_empty();
    string_append_c(res, "    .data\n");
    string_append(res, globl_buffer);
    string_append_c(res, "\n    .section    .rodata\n");
    string_append(res, ro_buffer);
    string_append_c(res, "\n    .text\n");
    string_append(res, meat_and_potatos);
    string_free(meat_and_potatos);
    code_gen_free_statics();
    return res;
}




///////////////////////////////////////////////
//          DAG code gen begin
///////////////////////////////////////////////

void code_add_dag_gen(string *s, struct dag_node *dag, struct SymbolTable* st, int stk_lcls){
    auto cod = dag_code_gen(dag, st, stk_lcls);
    string_append(s, cod);
    string_free(cod);
}

void code_add_print_c(string * res, char * s_print_s, char * other){
    char * print_code = calloc(100, sizeof(char));
    if(other){
        sprintf(print_code, "\
    leaq %s(%%rip), %%rdi\n\
    movq %s, %%rsi\n\
    movq $0, %%rax\n\
    call printf@plt\n", s_print_s, other);
    }else{
        sprintf(print_code, "\
    leaq %s(%%rip), %%rdi\n\
    movq $0, %%rax\n\
    call printf@plt\n", s_print_s);
    }
    string_append_c(res, print_code);
    free(print_code);
}

void code_add_print_d(string * res, char * s_print_s, struct dag_node *dag, int stk_lcls){
    if(dag){
        code_add_print_c(res, s_print_s, scratch_register(dag->ershov, stk_lcls));
    }else{
        code_add_print_c(res, s_print_s, 0);
    }
}
void code_add_print_literal(string * res, char * lit){
    auto print_l = add_ro(ro_buffer, ro_string_holder, lit);
    auto print_l_s = get_label(print_l);
    code_add_print_c(res, print_l_s, 0);
    free(print_l_s);
}

string * create_print_format_helper(struct ast_vtype * type){
    auto format = string_create_empty();
    if(type->kind == VTYPE_STRING){
        string_append_c(format, "%s");
    }else if(type->kind == VTYPE_CHAR){
        string_append_c(format, "%c");
    }else if(type->kind == VTYPE_INT){
        string_append_c(format, "%d");
    }else if(type->kind == VTYPE_BOOL){
        string_append_c(format, "%d");
    }else if(type->kind == VTYPE_ARRAY){
        string_append_c(format, "{");
        auto sub_format = create_print_format_helper(type->subtype);
        string_append(format, sub_format);
        for(int i = 1; i < type->arr_sz; i++){
            string_append_c(format, ",");
            string_append(format, sub_format);
        }
        string_free(sub_format);
        string_append_c(format, "}");
    }else{
        fprintf(stderr, "Cannot print functions or void type.\n");
        exit(1);
    }
    return format;
}


int create_print_format(struct ast_vtype * type){
    auto format = create_print_format_helper(type);
    string_append_c(format, "\\n");
    int s_print_label = add_ro(ro_buffer, ro_string_holder,  format->charr);
    string_free(format);
    return s_print_label;

}


string * code_gen_cf_print(struct dag_node * dag, struct SymbolTable * st, int stk_lcls){
    auto res = string_create_empty();
    if(dag->type->kind != VTYPE_ARRAY){
        int s_print_label = create_print_format(dag->type);
        char * s_print_s = get_label(s_print_label);
        code_add_dag_gen(res, dag, st, stk_lcls);
        code_add_print_d(res, s_print_s, dag, stk_lcls);
        free(s_print_s);
    }else if(dag->kind == DAG_NAME && dag->type->kind == VTYPE_ARRAY){

        auto sym = sym_table_lookup(st, dag->name);
        auto total_size = sym->type->arr_sz * sym->type->arr_stride;
        int extra_args = total_size - 5;
        bool did_sub = false;
        {
            int s_print_label = create_print_format(dag->type);
            char * s_print_s = get_label(s_print_label);
            char sps_code[30] = "";
            sprintf(sps_code, "%s(%%rip)", s_print_s);
            code_op_cc(res, "leaq", sps_code, "%rdi");
            free(s_print_s);
        }
        if(sym->scope_level == 0){
            for(int i = 0; i < total_size; i++){
                char sym_p_code[30] = "";
                sprintf(sym_p_code, "%s + %d(%%rip)", sym->name, i);
                if(i >= 5){
                    if(!did_sub && extra_args%2){
                        code_op_cc(res, "subq", "$8", "%rsp");
                        did_sub = true;
                    }
                    code_op_cc(res, "pushq", sym_p_code, 0);
                }else{
                    code_op_cc(res, "movq", sym_p_code, param_registers_code_gen[i+1]);
                }
            }
        }else if(!(sym->is_param)){
            for(int i = total_size-1; i >= 0; i--){
                char sym_p_code[30] = "";
                sprintf(sym_p_code, "-%d(%%rbp)", 56 + sym->reg_pos * 8 + i * 8);
                if(i >= 5){
                    if(!did_sub && extra_args%2){
                        code_op_cc(res, "subq", "$8", "%rsp");
                        did_sub = true;
                    }
                    code_op_cc(res, "pushq", sym_p_code, 0);
                }else{
                    code_op_cc(res, "movq", sym_p_code, param_registers_code_gen[i+1]);
                }
            }
        }else{
            fprintf(stderr, "Function params aren't allowed yet :/\n");
            exit(1);
        }
        code_op_cc(res, "movq", "$0", "%rax");
        code_op_cc(res, "call", "printf@PLT",0);

        if(extra_args > 0){
            extra_args += extra_args%2;
            char restore_sp[30] = "";
            sprintf(restore_sp, "    addq $%d, %%rsp\n", extra_args * 8);
            string_append_c(res, restore_sp);
        }
    }else{
        fprintf(stderr, "Don't support printing literal arrays yet :(.\n");
        exit(1);
    }
    return res;
}

string *dag_op_arith_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls){
    string * res = string_create_empty();

    if(dag->kind == DAG_NEGATION){
        code_add_dag_gen(res, dag->left, st, stk_lcls);
        code_op_dc(res, "negq", dag->left, 0, stk_lcls);
        return res;
    }

    if(dag->kind == DAG_BITNOT){
        code_add_dag_gen(res, dag->left, st, stk_lcls);
        code_op_dc(res, "notq", dag->left, 0, stk_lcls);
        return res;
    }

    string * actual_command = string_create_empty();
    switch(dag->kind){
        case DAG_MULT:
            string_append_c(actual_command, "imulq");
            break;
        case DAG_PLUS:
            string_append_c(actual_command, "addq");
            break;
        case DAG_BITOR:
            string_append_c(actual_command, "orq");
            break;
        case DAG_BITXOR:
            string_append_c(actual_command, "xorq");
            break;
        case DAG_BITAND:
            string_append_c(actual_command, "andq");
            break;
        case DAG_MINUS:
            string_append_c(actual_command, "subq");
            break;
        case DAG_DIV:
        case DAG_MOD:
            string_append_c(actual_command, "idivq");
            break;
        default:
    }
    string * l_s = dag_code_gen(dag->left, st, stk_lcls);
    string * r_s = dag_code_gen(dag->right, st, stk_lcls);

    if(dag->kind == DAG_DIV || dag->kind == DAG_MOD){
        string_append(res, l_s);
        code_op_dc(res,"movq", dag->left, "%rax", stk_lcls);
        string_append(res, r_s);
        //cqto = cto = quad to oct: sign extend %rax to %(rdx:rax)
        //idivq is basically division of an oct (128-bit) by a quad
        string_append_c(res, "    cqto\n");
        code_op_dc(res,actual_command->charr, dag->right, 0, stk_lcls);
        if(dag->kind == DAG_DIV){
            code_op_cd(res,"movq", "%rax", dag, stk_lcls);
        }else{
            code_op_cd(res,"movq", "%rdx", dag, stk_lcls);
        }
    }else if(dag->left->ershov > dag->right->ershov){
        string_append(res, l_s);
        string_append(res, r_s);
        code_op_dd(res,actual_command->charr, dag->right, dag, stk_lcls);
    }else if (dag->left->ershov < dag->right->ershov){
        //Have to add an extra mov instruction due to noncommutative operations
        string_append(res, r_s);
        string_append(res, l_s);
        code_op_dd(res,actual_command->charr, dag, dag->left, stk_lcls);
        code_op_dd(res,"movq", dag->left, dag, stk_lcls);
    }else{
        string_append(res, l_s);
        code_op_dd(res,"movq", dag->left, dag, stk_lcls);
        string_append(res, r_s);
        code_op_dd(res,actual_command->charr, dag->right, dag, stk_lcls);
    }
    string_free(l_s);
    string_free(r_s);
    return res;
}

string * dag_op_bool_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls){
    string * res = string_create_empty();
    if(dag->kind == DAG_NOT){
        code_add_dag_gen(res, dag->left, st, stk_lcls);
        code_op_cd(res, "xorq", "$1", dag->left, stk_lcls);
        return res;
    }
    string * actual_command = string_create_empty();
    switch(dag->kind){
        case DAG_GREATER:
            string_append_c(actual_command, "setg");
            break;
        case DAG_LESS:
            string_append_c(actual_command, "setl");
            break;
        case DAG_GEQ:
            string_append_c(actual_command, "setge");
            break;
        case DAG_LEQ:
            string_append_c(actual_command, "setle");
            break;
        case DAG_EQUALITY:
            string_append_c(actual_command, "setne");
            break;
        case DAG_INEQUALITY:
            string_append_c(actual_command, "sete");
            break;
        default:
    }
    string * l_s = dag_code_gen(dag->left, st, stk_lcls);
    string * r_s = dag_code_gen(dag->right, st, stk_lcls);
    if(dag->kind == DAG_AND){
        int condn_label = new_label();
        int done_label = new_label();
        auto condn_s = get_label(condn_label);
        auto done_s = get_label(done_label);
        string_append(res, l_s);
        code_op_dc(res, "movq", dag->left, "%rax", stk_lcls);
        code_op_cc(res, "testq","%rax", "%rax");
        code_op_cc(res, "jz",condn_s, 0);
        string_append(res, r_s);
        code_op_dc(res, "movq", dag->right, "%rax", stk_lcls);
        code_op_cc(res, "testq","%rax", "%rax");
        code_op_cc(res, "jz",condn_s, 0);
        code_op_cd(res, "movq", "$1", dag, stk_lcls);
        code_op_cc(res, "jmp", done_s,0);
        code_label_c(res, condn_s);
        code_op_cd(res, "movq", "$0", dag, stk_lcls);
        code_label_c(res, done_s);
        free(condn_s);
        free(done_s);
    }else if(dag->kind == DAG_OR){
        int condn_label = new_label();
        int done_label = new_label();
        auto condn_s = get_label(condn_label);
        auto done_s = get_label(done_label);
        string_append(res, l_s);
        code_op_dc(res, "movq", dag->left, "%rax", stk_lcls);
        code_op_cc(res, "testq","%rax", "%rax");
        code_op_cc(res, "jnz",condn_s, 0);
        string_append(res, r_s);
        code_op_dc(res, "movq", dag->right, "%rax", stk_lcls);
        code_op_cc(res, "testq","%rax", "%rax");
        code_op_cc(res, "jnz",condn_s, 0);
        code_op_cd(res, "movq", "$0", dag, stk_lcls);
        code_op_cc(res, "jmp", done_s,0);
        code_label_c(res, condn_s);
        code_op_cd(res, "movq", "$1", dag, stk_lcls);
        code_label_c(res, done_s);
        free(condn_s);
        free(done_s);
    }else {
        if(dag->left->ershov > dag->right->ershov){
            string_append(res, l_s);
            string_append(res, r_s);
            code_op_cc(res, "xorq", "%rax", "%rax");
            code_op_dd(res, "cmpq", dag->right, dag->left, stk_lcls);
        }else if (dag->left->ershov < dag->right->ershov){
            //Have to add an extra mov instruction due to noncommutative operations
            string_append(res, r_s);
            string_append(res, l_s);
            code_op_cc(res, "xorq", "%rax", "%rax");
            code_op_dd(res, "cmpq", dag->right, dag->left, stk_lcls);
        }else{
            string_append(res, l_s);
            code_op_dd(res,"movq", dag->left, dag, stk_lcls);
            string_append(res, r_s);
            code_op_cc(res, "xorq", "%rax", "%rax");
            code_op_dd(res, "cmpq", dag->right, dag, stk_lcls);
        }
        code_op_cc(res, actual_command->charr, "%al",0);
        code_op_cc(res, "movzbq", "%al", "%rax");
        code_op_cd(res, "movq", "%rax", dag, stk_lcls);
    }
    string_free(l_s);
    string_free(r_s);
    return res;

}
string *dag_code_gen(struct dag_node *dag, struct SymbolTable *st, int stk_lcls){
    /*
     * Meat of the code gen
     * The result of a dag should be in scratch_register(dag->ershov)
     */
    string *res = 0;
    switch(dag->kind){
        case DAG_LIT_BOOL:
        case DAG_LIT_INT:
        case DAG_LIT_CHAR:{
                res = string_create_empty();
                char int_res[25] = "";
                sprintf(int_res, "    movq $%d, %%rbx\n", dag->lit_int);
                string_append_c(res, int_res);
                return res;
        }
        case DAG_LIT_STRING:{
                res = string_create_empty();
                int s_string_label = add_ro(ro_buffer, ro_string_holder,  dag->lit_string);
                char int_res[50] = "";
                sprintf(int_res, "    leaq .L%d(%%rip), %%rbx\n", s_string_label);
                string_append_c(res, int_res);
                return res;
        }
        case DAG_NAME:{
                res = string_create_empty();
                code_op_nc(res, "movq", dag->name, "%rbx", st);
                return res;
        }
        case DAG_DEREF:{
            /*
             * Dereferencing arrays.
             * Worth pointing out that, without pointers, nested arrays are no different from normal arrays.
             */

            res = string_create_empty();
            code_add_dag_gen(res, dag->right, st, stk_lcls);
            //TODO Either keep or put arrays on stack in reverse order to remove negq.
            auto sym = sym_table_lookup(st, dag->left->name);
            if(sym->scope_level == 0){
                code_op_cc(res, "leaq", symbol_code_gen(sym), "%rax");
                char result[30] = "";
                sprintf(result, "    movq (%%rax, %s, 8), %s\n", scratch_register(dag->right->ershov, stk_lcls), scratch_register(dag->ershov, stk_lcls));
                string_append_c(res, result);
            }else{
                code_op_dc(res, "negq", dag->right, 0, stk_lcls);
                code_op_cd(res, "movq", symbol_code_gen_with_offset(sym, dag->right, stk_lcls), dag, stk_lcls);
            }
            printf("%s", res->charr);
            return res;
        }
        case DAG_FN_CALL:{
            /*
             * TODO this is wrong: %rdi etc. (and stack) get clobbered if the call args involve another fn call
             * to fix: always put it on stack
             *
             *
             * 1. Make less spaghetti
             * 2. Do pushes (and the whole thing) in reverse order of params (this is easier once all put on stack first)
             * 3. Allocate on stack at the beginning (via the fn_call_args member; need to add to context everywhere)
             */

            res = string_create_empty();
            //Phase 1. Compute fn args and place on stack
            int extra_args = 0;
            int arg_reg = 0;
            auto args = dag->right;
            while(args){
                code_add_dag_gen(res, args->left, st, stk_lcls);
                char param_arg_location[20] = "";
                sprintf(param_arg_location, "%d(%%rbp)", -(56 + (stk_lcls - arg_reg)*8));
                code_op_dc(res, "movq", args->left, param_arg_location, stk_lcls);
                args = args->right;
                //Since I need 16 byte alignment, have to check if extra_args is odd
                if(arg_reg >= 6){
                    extra_args++;
                }
                arg_reg++;
            }
            //Stack must be "off by 8 bytes" in 16 byte frame
            //since call adds an extra 8 for return address
            if(extra_args%2){
                code_op_cc(res, "subq", "$8", "%rsp");
            }
            //Phase 2. Put fn args with pushes and so on
            arg_reg = 0;
            args = dag->right;
            while(args){
                char param_arg_location[20] = "";
                sprintf(param_arg_location, "%d(%%rbp)",  -(56 + (stk_lcls - arg_reg)*8));
                if(arg_reg >= 6){
                    code_op_cc(res, "pushq", param_arg_location, 0);
                }else{
                    code_op_cc(res, "movq", param_arg_location, param_registers_code_gen[arg_reg]);
                }
                arg_reg++;
                args = args->right;
            }
            args = dag->right;

            code_op_cc(res, "pushq","%r10",0);
            code_op_cc(res, "pushq","%r11",0);
            code_op_cc(res, "call",dag->left->name,0);
            code_op_cc(res, "popq","%r11",0);
            code_op_cc(res, "popq","%r10",0);

            if(extra_args > 0){
                extra_args += extra_args%2;
                char restore_sp[30] = "";
                sprintf(restore_sp, "    addq $%d, %%rsp\n", extra_args * 8);
                string_append_c(res, restore_sp);
            }

            code_op_cd(res, "movq","%rax",dag, stk_lcls);
            return res;
        }
        case DAG_ASSIGN:
            res = string_create_empty();
            if(dag->left->kind == DAG_DEREF){
                auto l = dag_code_gen(dag->left->right, st, stk_lcls);
                auto r = dag_code_gen(dag->right, st, stk_lcls);
                if(dag->left->ershov > dag->right->ershov){
                    string_append(res,l);
                    string_append(res,r);
                }else if(dag->left->ershov < dag->right->ershov){
                    string_append(res,r);
                    string_append(res,l);
                }else{
                    string_append(res,l);
                    code_op_dd(res, "movq", dag->left, dag, stk_lcls);
                    string_append(res,r);
                }
                //TODO Either keep or put arrays on stack in reverse order to remove negq.
                auto sym = sym_table_lookup(st, dag->left->left->name);
                if(sym->scope_level == 0){
                    code_op_cc(res, "leaq", symbol_code_gen(sym), "%rax");
                    char result[30] = "";
                    sprintf(result, "    movq %s, (%%rax, %s, 8)\n", scratch_register(dag->right->ershov, stk_lcls), scratch_register(dag->ershov, stk_lcls));
                    string_append_c(res, result);
                }else{
                    code_op_dc(res, "negq", dag, 0, stk_lcls);
                    code_op_dc(res, "movq", dag->right, symbol_code_gen_with_offset(sym, dag, stk_lcls), stk_lcls);
                }
            }else{
                code_add_dag_gen(res, dag->right, st, stk_lcls);
                code_op_dn(res, "movq", dag->right, dag->left->name, st, stk_lcls);
            }
            return res;
        case DAG_INCREMENT:
        case DAG_DECREMENT:
            res = string_create_empty();
            code_add_dag_gen(res, dag->left, st, stk_lcls);
            auto goer = dag->left;
            while(goer->kind != DAG_NAME) {
                goer = goer->left;
            }
            if(dag->kind == DAG_INCREMENT) {
                code_op_cn(res, "addq", "$1", goer->name, st);
            }else {
                code_op_cn(res, "subq", "$1", goer->name, st);
            }
            return res;
        case DAG_NOT:
        case DAG_AND:
        case DAG_OR:
        case DAG_EQUALITY:
        case DAG_INEQUALITY:
        case DAG_LESS:
        case DAG_GREATER:
        case DAG_GEQ:
        case DAG_LEQ:
            return dag_op_bool_code_gen(dag, st, stk_lcls);
        case DAG_ARR_MEMBER:
            return string_create_empty();
        default:
            return dag_op_arith_code_gen(dag, st, stk_lcls);
    }
    return res;
}


///////////////////////////////////////////////
//          DAG code gen end
///////////////////////////////////////////////



string *stmt_code_gen(struct control_flow_node *cf, struct SymbolTable *st, char* fn_id, int stk_lcls){
    /*
     * Stupidly I've decided to make things hard and use the control flow.
     * So I've had to introduce ENDIF, ENDELSE, ENDFOR and GOTO nodes.
     */
    if(!cf) {
        if(sym_table_lookup(st, fn_id)->type->subtype->kind != VTYPE_VOID){
            fprintf(stderr, "Warning: hit the end of a control loop without returning a value in a non-void function.\n");
        }
        return string_create_empty();
    }
    switch(cf->kind){
        case CF_DECL:{
            auto res = decl_code_gen(cf, st, stk_lcls);
            auto r = stmt_code_gen(cf->next, st, fn_id, stk_lcls);
            string_append(res, r);
            string_free(r);
            return res;
        }
        case CF_NOP:
            return stmt_code_gen(cf->next, st, fn_id, stk_lcls);
        case CF_DAG:
            if(cf->false_next){
                auto res = string_create_empty();
                //BUG something's wrong with the exit scoping here.
                if(cf->false_next->kind == CF_ENDFOR){
                    int cond_label = new_label();
                    int else_label = new_label();
                    stacki_push(cond_label_stack, cond_label);
                    auto l = stmt_code_gen(cf->next, st, fn_id, stk_lcls);
                    auto r = stmt_code_gen(cf->false_next, st, fn_id, stk_lcls);
                    auto d = dag_code_gen(cf->dag, st, stk_lcls);


                    code_label_i(res, cond_label);
                    string_append(res, d);
                    code_op_dc(res, "movq", cf->dag, "%rax", stk_lcls);
                    code_op_cc(res, "testq", "%rax", "%rax");
                    code_jump_label_i(res, "jz", else_label);
                    string_append(res, l);
                    code_label_i(res, else_label);
                    string_append(res, r);

                    string_free(d);
                    string_free(l);
                    string_free(r);
                }else{
                    int else_label = new_label();
                    int done_label = new_label();
                    stacki_push(done_label_stack, done_label);
                    auto l = stmt_code_gen(cf->next, st, fn_id, stk_lcls);
                    auto r = stmt_code_gen(cf->false_next, st, fn_id, stk_lcls);
                    auto d = dag_code_gen(cf->dag, st, stk_lcls);


                    string_append(res, d);
                    code_op_dc(res, "movq", cf->dag, "%rax", stk_lcls);
                    code_op_cc(res, "testq", "%rax", "%rax");
                    code_jump_label_i(res, "jz", else_label);
                    string_append(res, l);
                    code_jump_label_i(res, "jmp", done_label);
                    code_label_i(res, else_label);
                    string_append(res, r);

                    string_free(d);
                    string_free(l);
                    string_free(r);
                }
                return res;
            }else{
                auto res = dag_code_gen(cf->dag, st, stk_lcls);

                auto l = stmt_code_gen(cf->next, st, fn_id, stk_lcls);
                string_append(res, l);
                string_free(l);

                return res;
            }
        case CF_RET:{
                auto res = dag_code_gen(cf->dag, st, stk_lcls);
                code_op_dc(res, "movq", cf->dag, "%rax", stk_lcls);
                string_append_c(res, "    jmp ");
                string_append_c(res, fn_id);
                string_append_c(res, "_EP\n");
                return res;
        }
        case CF_ENDFOR:
            stacki_pop(cond_label_stack);
            return stmt_code_gen(cf->next, st, fn_id, stk_lcls);
        case CF_ENDELSE:{
            auto res = string_create_empty();
            code_label_i(res, stacki_pop(done_label_stack));
            auto s = stmt_code_gen(cf->next->next, st, fn_id, stk_lcls);
            string_append(res, s);
            string_free(s);
            return res;
        }
        case CF_ENDIF:{
            return string_create_empty();
        }
        case CF_GOTO:{
            auto res = string_create_empty();
            code_jump_label_i(res, "jmp", stacki_top(cond_label_stack));
            return res;
        }
        case CF_SCOPE_ENTER:
            sym_table_enter(st);
            return stmt_code_gen(cf->next, st, fn_id, stk_lcls);
        case CF_SCOPE_EXIT:
            sym_table_pop(st);
            return stmt_code_gen(cf->next, st, fn_id, stk_lcls);
        case CF_PRINT:
            if(cf->dag->type->kind == VTYPE_INT || cf->dag->type->kind == VTYPE_STRING || cf->dag->type->kind == VTYPE_ARRAY || cf->dag->type->kind){
                auto res = code_gen_cf_print(cf->dag, st, stk_lcls);
                auto s = stmt_code_gen(cf->next, st, fn_id, stk_lcls);
                string_append(res, s);
                string_free(s);
                return res;
            }
            else{
                fprintf(stderr, "Don't print functions.\n");
                exit(1);
                return 0;
            }
        default:
            fprintf(stderr, "Apparently I made a statement but haven't defined it yet.\n");
            exit(1);
            return 0;
    }
}


void code_gen_local_arr_member_helper(string * res, struct Symbol * sym, struct dag_node * dag, int offset, struct SymbolTable * st, int stk_lcls){
    if(dag->kind == DAG_ARR_MEMBER){
        code_gen_local_arr_member_helper(res, sym, dag->left, offset, st, stk_lcls);
        if(dag->right) code_gen_local_arr_member_helper(res, sym, dag->right, offset + dag->type->arr_stride, st, stk_lcls);
    }else{
        char to_add[30] = "";
        code_add_dag_gen(res, dag, st, stk_lcls);
        sprintf(to_add, "    movq %s, -%d(%%rbp)\n", scratch_register(dag->ershov, stk_lcls), 56 + (offset + sym->reg_pos)*8);
        string_append_c(res, to_add);
    }
}

string *decl_code_gen(struct control_flow_node *cf, struct SymbolTable *st, int stk_lcls){
    auto decl = cf->decl;
    auto res = string_create_empty();
    if(decl->type->kind == VTYPE_FN && decl->code){
        /*
         * Have to set up stack frame.
         * For now, I'm just gonna push all the registers
         */
        code_op_cc(res, ".globl", decl->id, 0);
        code_label_c(res, decl->id);
        string_append_c(res, fn_intro1);
        string_append_c(res, fn_intro2);

        /*
         * Function body...
         * 1.a. Subtract 8 * (max(0, fn_ershov - 7) + stk_lcls) from rsp
         * 1.b. Align this with 16 byte frame.
         * 2. Load params into symbol table.
         * 3. Generate code for each statement.
         */
        sym_list_insert(st, decl->id, 0, decl->type, 0, 1);
        sym_table_enter(st);

        //TODO
        //Either..
        //1. Put fcallargs in the function calls below, or...
        //2.  pass fdecls + fcallargs, or
        //3. What I currently have in fn call: pushing onto stack each time
        auto fdecls = cf->fn_decls;
        auto fershov = cf->fn_ershov;
        auto fcallargs = cf->fn_call_args;
        auto fstack_lcls = fdecls + fcallargs;
        char change_rsp[20] = "";
        unsigned stack_frame_offset =  fstack_lcls+ (fershov >= 7 ? fershov - 7 : 0);
        //TODO figure out why I added 16
        //e.g. what if there are no fdecls? the pushes subtract 48, after that is 56
        stack_frame_offset = 16 + ((stack_frame_offset)/2)*16;
        if(stack_frame_offset){
            sprintf(change_rsp, "    subq $%d, %%rsp\n", stack_frame_offset);
            string_append_c(res, change_rsp);
        }

        auto goer = decl->type->args;
        while(goer){
            sym_list_insert(st, goer->name, st->sz-1, goer->type, true, 0);
            goer = goer->next;
        }


        auto res_code = stmt_code_gen(cf->code, st, decl->id, fstack_lcls);
        string_append(res, res_code);
        string_free(res_code);

        string_append_c(res, decl->id);
        string_append_c(res, "_EP:\n");
        if(stack_frame_offset){
            sprintf(change_rsp, "    addq $%d, %%rsp\n", stack_frame_offset);
            string_append_c(res, change_rsp);
        }
        string_append_c(res, fn_outro);
        sym_table_pop(st);
    }else if(decl->type->kind != VTYPE_FN && st->sz == 1) {
        sym_list_insert(st, decl->id, 0, decl->type, false, 0);
        add_global_decl(decl);
    }
    else{
        sym_list_insert(st, decl->id, st->sz-1, decl->type, false, 0);
        if(decl->val){
            if(decl->type->kind == VTYPE_ARRAY){
                auto sym = sym_table_lookup(st, decl->id);
                code_gen_local_arr_member_helper(res, sym, cf->dag, 0, st, stk_lcls);
            }else{
                auto assign_res = dag_code_gen(cf->dag, st, stk_lcls);
                string_append(res, assign_res);
                string_free(assign_res);
                code_op_dn(res, "movq", cf->dag, decl->id, st, stk_lcls);
            }
        }
    }
    return res;
}



////////////////////////////////////////////////////////////////


void add_global_decl(struct ast_decl *decl){
    string_append_c(globl_buffer, "    .globl ");
    string_append_c(globl_buffer, decl->id);
    string_append_c(globl_buffer, "\n");
    switch(decl->type->kind){
        case VTYPE_BOOL:
            char bool_res[2] = "";
            string_append_c(globl_buffer, decl->id);
            string_append_c(globl_buffer, ":\n    .quad ");
            bool_res[0] = decl->val->lit_int + '0';
            string_append_c(globl_buffer, bool_res);
            string_append_c(globl_buffer, "\n");
            break;
        case VTYPE_INT:
            string_append_c(globl_buffer, decl->id);
            string_append_c(globl_buffer, ":\n    .quad ");
            char int_res[15] = "";
            sprintf(int_res, "%d", decl->val->lit_int);
            string_append_c(globl_buffer, int_res);
            string_append_c(globl_buffer, "\n");
            break;
        case VTYPE_CHAR:
            string_append_c(globl_buffer, decl->id);
            string_append_c(globl_buffer, ":\n    .quad ");
            char char_res[8] = "";
            sprintf(int_res, "%d", decl->val->lit_int);
            string_append_c(globl_buffer, char_res);
            string_append_c(globl_buffer, "\n");
            break;
        case VTYPE_STRING:
            string_append_c(ro_buffer, decl->id);
            string_append_c(ro_buffer, ":\n    .string \"");
            string_append_c(ro_buffer, decl->val->lit_string);
            string_append_c(ro_buffer, "\"\n");
            break;
        case VTYPE_ARRAY:
            //TODO fix for nested arrays
            string_append_c(globl_buffer, decl->id);
            string_append_c(globl_buffer, ":\n");
            if(decl->type->subtype->kind == VTYPE_STRING){
                auto goer = decl->val;
                while(goer){
                    auto s_label = add_ro(ro_buffer,ro_string_holder, goer->left->lit_string);
                    auto s_label_s = get_label(s_label);
                    code_op_cc(globl_buffer, ".quad", s_label_s,0);
                    goer = goer->right;
                }

            }else{
                auto goer = decl->val;
                while(goer){
                    char int_res[15] = "";
                    sprintf(int_res, "%d", goer->left->lit_int);
                    code_op_cc(globl_buffer, ".quad", int_res, 0);
                    goer = goer->right;
                }
            }
        default:
            return;
    }
}
