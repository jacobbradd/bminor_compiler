
#include <stdio.h>
#include <stdlib.h>

#include "dag_simple.h"
#include "sym_table.h"
#include "cg_helper.h"
#include "data_types/string_t.h"
#include "data_types/map.h"

static int label_counter = 0;
static char *registers[] = {"%rbx", "%r15", "%r14", "%r13", "%r12", "%r11", "%r10"};
static char *param_registers[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
///////////////////////////////////////////////
// Many helper functions
///////////////////////////////////////////////



void code_op_dn(string *s, char *op, struct dag_node *dag1, char* name2, struct SymbolTable *st, int stk_lcls){
    code_op_cn(s, op, scratch_register(dag1->ershov, stk_lcls), name2, st);

}
void code_op_cn(string *s, char *op, char *val, char* name2, struct SymbolTable *st){
    code_op_cc(s, op, val, symbol_code_gen(sym_table_lookup(st, name2)));
}
void code_op_nc(string *s, char *op, char *name1, char* dest, struct SymbolTable *st){
    code_op_cc(s, op, symbol_code_gen(sym_table_lookup(st, name1)),dest);
}

void code_op_dc(string * s, char* op, struct dag_node * dag1, char *dest, int stk_lcls){
    code_op_sc(s, op, dag1->ershov, dest, stk_lcls);
}
void code_op_cd(string * s, char* op, char* val, struct dag_node * dag2, int stk_lcls){
    if(dag2){
        code_op_cs(s, op, val, dag2->ershov, stk_lcls);
    }else{
        code_op_cs(s, op, val, -1, stk_lcls);
    }
}
void code_op_dd(string * s, char* op, struct dag_node * dag1, struct dag_node * dag2, int stk_lcls){
    if(dag2){
        code_op_ss(s, op, dag1->ershov, dag2->ershov, stk_lcls);
    }else{
        code_op_ss(s, op, dag1->ershov, -1, stk_lcls);
    }
}

void code_op_sc(string * s, char* op, int scratch1, char *dest, int stk_lcls){
    code_op_cc(s, op, scratch_register(scratch1, stk_lcls), dest);
}

void code_op_cs(string * s, char* op, char* val, int scratch2, int stk_lcls){
    if(scratch2 >= 0){
        code_op_cc(s, op, val, scratch_register(scratch2, stk_lcls));
    }else{
        code_op_cc(s, op, val, 0);
    }
}
void code_op_ss(string * s, char* op, int scratch1, int scratch2, int stk_lcls){
    if(scratch2 >= 0){
        code_op_cc(s, op, scratch_register(scratch1, stk_lcls), scratch_register(scratch2, stk_lcls));
    }else{
        code_op_cc(s, op, scratch_register(scratch1, stk_lcls), 0);
    }
}
void code_op_cc(string * s, char * op, char* val, char *dest){
    string_append_c(s, "    ");
    string_append_c(s, op);
    if(val){
        string_append_c(s, " ");
        string_append_c(s, val);
        if(dest){
            string_append_c(s, ", ");
            string_append_c(s, dest);
        }
    }
    string_append_c(s, "\n");
}

void code_label_c(string *s, char* label){
    string_append_c(s, label);
    string_append_c(s, ":\n");
}

void code_label_i(string *s, int label){
    string_add_label(s, label);
    string_append_c(s, ":\n");
}

void code_jump_label_c(string *res, char* op, char* label){
    string_append_c(res, "    ");
    string_append_c(res,  op);
    string_append_c(res, " ");
    string_append_c(res, label);
    string_append_c(res, "\n");
}

void code_jump_label_i(string *res, char* op, int label){
    string_append_c(res, "    ");
    string_append_c(res,  op);
    string_append_c(res, " ");
    string_add_label(res, label);
    string_append_c(res, "\n");
}
///////////////////////////////////////////////
///////////////////////////////////////////////





////////////////////////////////////////////////////////////////

char *symbol_code_gen(struct Symbol *s){
    if(s->scope_level == 0){
        /*
         * As usual, things are a little more complicated.
         * In a PIE, you have to use rip offset
         */
        char *res = calloc(15, sizeof(char));
        sprintf(res, "%s(%%rip)", s->name);
        return res;
    }else if(!(s->is_param)){
        char *res = calloc(15, sizeof(char));
        sprintf(res, "-%d(%%rbp)", 56 + s->reg_pos * 8);
        return res;
    }else{
        if(s->param_pos <= 6){
            return param_registers[s->param_pos-1];
        }else{
            char *res = calloc(15, sizeof(char));
            //TODO figure out the actual value here... (not 32 in general)
            sprintf(res, "%d(%%rbp)", 32 + (s->param_pos - 7) * 8);
            return res;
        }
    }
    return 0;
}

char *symbol_code_gen_with_offset(struct Symbol *s, struct dag_node * dag, int stk_lcls){
    auto offset = scratch_register(dag->ershov, stk_lcls);
    if(s->scope_level == 0){
        /*
         * As usual, things are a little more complicated.
         * In a PIE, you have to use rip offset
         *  TODO so... they don't let you do this???
         * Anyway here's a position-dependent approach
         */
        char *res = calloc(15, sizeof(char));
        sprintf(res, "%s(, %s, 8)", s->name, offset);
        return res;
    }else if(!(s->is_param)){
        char *res = calloc(15, sizeof(char));
        sprintf(res, "-%d(%%rbp, %s, 8)", 56 + s->reg_pos * 8, offset);
        return res;
    }else{
        if(s->param_pos <= 6){
            return param_registers[s->param_pos-1];
        }else{
            char *res = calloc(15, sizeof(char));
            //why'd I use 32 here?
            sprintf(res, "%d(%%rbp, %s, 8)", 32 + (s->param_pos - 7) * 8, offset);
            return res;
        }
    }
    return 0;
}

char *scratch_register(int pos, int stk_lcls){
    if(pos <= 7) return registers[pos-1];
    char *res_c = calloc(15, sizeof(char));
    sprintf(res_c, "-%d(%%rbp)", 56+stk_lcls*8+(pos-7)*8);
    return res_c;
}


int new_label(){
    label_counter++;
    return label_counter;
}

char * get_label(int label_num){
    char *label = calloc(10, sizeof(char));
    sprintf(label, ".L%d", label_num);
    return label;
}

char * get_local_var(int pos){
    char *label = calloc(10, sizeof(char));
    sprintf(label, "%d(%%rsp)", -(56 + 8 * pos));
    return label;
}

void string_add_label(string *s, int label_num){
    auto lab = get_label(label_num);
    string_append_c(s, lab);
    free(lab);
}


void string_add_local_var(string *s, int label_num){
    auto lab = get_local_var(label_num);
    string_append_c(s, lab);
    free(lab);
}


int add_ro(string * ro_buffer, hash_int * ro_string_holder, char * s){
    if(hash_int_find(ro_string_holder, s)){
        return hash_int_get_val(ro_string_holder, s);
    }
    int res = new_label();
    char label[25] = "";
    sprintf(label, ".L%d:\n    .string \"", label_counter);
    string_append_c(ro_buffer, label);
    string_append_c(ro_buffer, s);
    string_append_c(ro_buffer, "\"\n");
    hash_int_insert(ro_string_holder, s, res);
    return res;
}
