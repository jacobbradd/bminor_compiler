/*
 * sem_analyser.c
 *
 * This performs (somewhat strict) type-checking on the ast.
 * Only thing it doesn't check is array bounds, due to the fact that it would require evaluation
 * TODO I've messed up array assignment bound checks. Maybe just don't let it not be equal.
 *
 *
 *
 */

#include "sem_analyser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ast.h"
#include "sym_table.h"

struct ast_vtype *sem_analyse_expr_arr_member(struct ast_expr *expr, struct SymbolTable *st);
struct ast_vtype *sem_analyse_expr(struct ast_expr *expr, struct SymbolTable *st);
void sem_analyse_stmt(struct ast_stmt *stmt, struct SymbolTable *st);
void sem_analyse_decl(struct ast_decl *decl, struct SymbolTable *st);

void sem_fail_print(const char * err, ...){
    va_list args;
    va_start(args, err);
    fprintf(stderr, "Semantic fail: ");
    vfprintf(stderr, err, args);
    fprintf(stderr, "\n");
    exit(1);
}

char expr_to_string[30][5] = {
    "","","++","--","!","~","-","*","%","/","+","-","<",">","<=",">=","==","!=","&","^", "|","&&","||",
    "="
};

void print_ast_expr(struct ast_expr *expr, bool to_error){
    auto f = stdout;
    if(to_error) f = stderr;
    switch(expr->kind){
        case EXPR_ARR_MEMBER:
            return; //TODO I've forgotten how I made this one
        case EXPR_ARR_ACCESS:
            fprintf(f, "[");
            print_ast_expr(expr->left, to_error);
            fprintf(f, "]");
            return;
        case EXPR_FN_CALL:
            if(expr->left) print_ast_expr(expr->left, to_error);
            fprintf(f, "(");
            if(expr->right) print_ast_expr(expr->right, to_error);
            fprintf(f, ")");
            return;
        case EXPR_FN_ARG:
            print_ast_expr(expr->left, to_error);
            if(expr->right){
                fprintf(f, ",");
                print_ast_expr(expr->right, to_error);
            }
            return;
        case EXPR_NAME:
            fprintf(f, "%s", expr->name);
            return;
        case EXPR_LIT_BOOL:
            fprintf(f, "%s", expr->lit_int ? "true" : "false");
            return;
        case EXPR_LIT_INT:
            fprintf(f, "%i", expr->lit_int);
            return;
        case EXPR_LIT_CHAR:
            fprintf(f, "%c", expr->lit_int);
            return;
        case EXPR_LIT_STRING:
            fprintf(f, "%s", expr->lit_string);
            return;
        case EXPR_NOT:
        case EXPR_BITNOT:
        case EXPR_NEGATION:
            fprintf(f, "%s", expr_to_string[expr->kind]);
            if(expr->left) print_ast_expr(expr->left, to_error);
            return;
        default:
            if(expr->left) print_ast_expr(expr->left, to_error);
            fprintf(f, "%s", expr_to_string[expr->kind]);
            if(expr->right) print_ast_expr(expr->right, to_error);
            return;
    }



}

void _report_failure(struct ast_expr *expr){
    fprintf(stderr, "Here's the expression that failed:\n");
    print_ast_expr(expr, true);
    fprintf(stderr, "\n");
    //Now print the expression...
    //What'd be better is if I kept track of the parent.
    //Then I can scale up a couple layers and print a bigger error.
    //(Of course, it's better to keep track of line data and print the whole source code line.)
}












struct ast_vtype *sem_analyse_expr_arr_member(struct ast_expr *expr, struct SymbolTable *st){
    struct ast_vtype * res = 0;
    if(expr->left){
        auto lt = sem_analyse_expr(expr->left, st);
        if(expr->right){
            auto rt = sem_analyse_expr(expr->right, st);
            if(!vtype_match(lt, rt->subtype)){
                sem_fail_print("Inconsistent array member types.");
            }
            auto az = rt->arr_sz + 1;
            ast_vtype_free(rt);
            res = create_ast_vtype_array(az, lt);
        }else{
            res = create_ast_vtype_array(1, lt);
        }
    }else{
        if(expr->right){
            auto rt = sem_analyse_expr(expr->right, st);
            auto az = rt->arr_sz + 1;
            ast_vtype_free(rt);
            res = create_ast_vtype_array(az, 0);
        }else{
            res = create_ast_vtype_array(1, 0);
        }
    }
    expr->type = ast_vtype_copy(res);
    return res;
}

struct ast_vtype *sem_analyse_expr_fn_call(struct ast_expr *expr, struct SymbolTable *st){
    struct ast_vtype *res = 0;
    auto fn = expr->left;
    auto found_s = sym_table_lookup(st, fn->name);
    if(found_s){
        res = ast_vtype_copy(found_s->type);
    }else{
        _report_failure(expr);
        sem_fail_print("No variable with name %s.", fn->name);
    }

    if(res->kind != VTYPE_FN){
        _report_failure(expr);
        sem_fail_print("You're trying to call something that isn't a function.");
    }
    auto goer_args = res->args;
    auto goer_call = expr->right;
    //Now arguments
    while(goer_args && goer_call){
        auto l = sem_analyse_expr(goer_call->left, st);
        if(!vtype_match(goer_args->type, l)){
            _report_failure(expr);
            sem_fail_print("Incorrect argument matching");
        }
        ast_vtype_free(l);
        goer_args = goer_args->next;
        goer_call = goer_call->right;
    }
    if(goer_args || goer_call){
        _report_failure(expr);
        sem_fail_print("Incorrect number of arguments in function.");
    }


    expr->type = ast_vtype_copy(res->subtype);
    return res->subtype;

}
char type_lookup[8][10] = {"bool", "int", "char", "string", "array", "function", "void"};



struct ast_vtype *sem_analyse_expr(struct ast_expr *expr, struct SymbolTable *st){
    //The actual type-checking
    //Have to traverse the tree and check everything

    //First of all, must resolve the types of the left and right
    //This means we must perform a post-order traversal
    //fn calls and array stuff are the exception really.
    if(expr->kind == EXPR_ARR_MEMBER){
        return sem_analyse_expr_arr_member(expr, st);
    }
    if(expr->kind == EXPR_INCREMENT || expr->kind == EXPR_DECREMENT){
        //This is to support double increment: a++ ++ = (a++)++
        auto goer = expr->left;
        while(goer->kind == EXPR_INCREMENT || goer->kind == EXPR_DECREMENT){
            goer = goer->left;
        }
        if(goer->kind != EXPR_NAME){
            sem_fail_print("Trying to increment/decrement a non-variable.");
        }
        struct ast_vtype *l =sem_analyse_expr(goer, st); //I'm lazy
        if(l->kind != VTYPE_INT){
            sem_fail_print("Trying to increment/decrement a non-integer variable.");
        }
        ast_vtype_free(l);
        return create_ast_vtype_normal(VTYPE_INT);
    }
    if(expr->kind == EXPR_FN_CALL){
        return sem_analyse_expr_fn_call(expr, st);
    }
    if(expr->kind == EXPR_ASSIGN){
        if(expr->left->kind != EXPR_NAME && expr->left->kind != EXPR_ARR_ACCESS){
            _report_failure(expr);
            sem_fail_print("Trying to assign to a non-variable.");
        }
        if(expr->left->kind == EXPR_ARR_ACCESS){
            //TODO "defining" arrays? I think they must be initialized always?
            struct ast_vtype *l = sem_analyse_expr(expr->left, st);
            struct ast_vtype *r = sem_analyse_expr(expr->right, st);
            auto found_s = sym_table_lookup(st, expr->left->left->name);
            sym_list_insert(st,
                            expr->left->left->name,
                            found_s->scope_level,
                            ast_vtype_copy(found_s->type),
                            found_s->is_param,
                            true);

            if(!vtype_match(l, r)){
                _report_failure(expr);
                sem_fail_print("Incompatible types in assignment.");
            }
            expr->type = ast_vtype_copy(l);
            return l;
        }else{
            struct ast_vtype *l =  sem_analyse_expr(expr->left, st);
            struct ast_vtype *r = sem_analyse_expr(expr->right, st);
            auto found_s = sym_table_lookup(st, expr->left->name);
            sym_list_insert(st,
                            expr->left->name,
                            found_s->scope_level,
                            ast_vtype_copy(found_s->type),
                            found_s->is_param,
                            true);
            //Should I support array/string assignment? Sure...
            if(!vtype_match(l, r)){
                _report_failure(expr);
                sem_fail_print("Incompatible types in assignment.");
            }
            expr->type = ast_vtype_copy(l);
            return l;
        }
    }

    struct ast_vtype *l = 0;
    struct ast_vtype *r = 0;
    struct ast_vtype *res = 0;
    if(expr->left) l = sem_analyse_expr(expr->left, st);
    if(expr->right) r = sem_analyse_expr(expr->right, st);
    //Now for the fun part.
    switch(expr->kind){
        case EXPR_ARR_ACCESS:
            //Note: cannot do static bound checking due to requiring code execution
            //This will have to be a runtime exception.
            if(r->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Array access index is not an integer.");
            }
            res = ast_vtype_copy(l->subtype);
            break;
        case EXPR_EQUALITY:
        case EXPR_INEQUALITY:
            //TODO only support for INT and CHAR, but could add string/array support
            if(l->kind != r->kind){
                sem_fail_print("Incompatible comparison types between %s and %s.",
                               type_lookup[l->kind],
                               type_lookup[r->kind]);
            }
            if(l->kind != VTYPE_INT && l->kind != VTYPE_CHAR){
                sem_fail_print("(In)equality not supported for type %s.", type_lookup[l->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_BOOL);
            break;
        case EXPR_LESS:
        case EXPR_GREATER:
        case EXPR_GEQ:
        case EXPR_LEQ:
            if(l->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Cannot do comparison with type %s.", type_lookup[l->kind]);
            }

            if(r->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Cannot do comparison with type %s.", type_lookup[r->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_BOOL);
            break;
        case EXPR_BITNOT:
        case EXPR_NEGATION:
            if(l->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Cannot do arithmetic with type %s.", type_lookup[l->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_INT);
            break;
        case EXPR_BITAND:
        case EXPR_BITOR:
        case EXPR_BITXOR:
        case EXPR_MULT:
        case EXPR_DIV:
        case EXPR_MOD:
        case EXPR_PLUS:
        case EXPR_MINUS:
            if(l->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Cannot do arithmetic with type %s.", type_lookup[l->kind]);
            }

            if(r->kind != VTYPE_INT){
                _report_failure(expr);
                sem_fail_print("Cannot do arithmetic with type %s.", type_lookup[r->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_INT);
            break;
        case EXPR_NOT:
            if(l->kind != VTYPE_BOOL){
                _report_failure(expr);
                sem_fail_print("Cannot do boolean logic with type %s.", type_lookup[l->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_BOOL);
            break;
        case EXPR_AND:
        case EXPR_OR:
            //Check if bools
            if(l->kind != VTYPE_BOOL){
                _report_failure(expr);
                sem_fail_print("Cannot do boolean logic with type %s.", type_lookup[l->kind]);
            }

            if(r->kind != VTYPE_BOOL){
                _report_failure(expr);
                sem_fail_print("Cannot do boolean logic  with type %s.", type_lookup[r->kind]);
            }
            res = create_ast_vtype_normal(VTYPE_BOOL);
            break;
        case EXPR_NAME:
            auto found_s = sym_table_lookup(st, expr->name);
            if(found_s == 0){
                _report_failure(expr);
                sem_fail_print("No variable with name %s.", expr->name);
            }
            if(found_s->is_defined == false && found_s->is_param == false){
                _report_failure(expr);
                sem_fail_print("Trying to use an undefined variable %s.", expr->name);
            }
            res = ast_vtype_copy(found_s->type);
            break;
        case EXPR_LIT_BOOL:
            res = create_ast_vtype_normal(VTYPE_BOOL);
            break;
        case EXPR_LIT_INT:
            res =  create_ast_vtype_normal(VTYPE_INT);
            break;
        case EXPR_LIT_CHAR:
            res = create_ast_vtype_normal(VTYPE_CHAR);
            break;
        case EXPR_LIT_STRING:
            res = create_ast_vtype_normal(VTYPE_STRING);
            break;
        default:
            break;
    }

    if(r) ast_vtype_free(r);
    if(l) ast_vtype_free(l);
    expr->type = ast_vtype_copy(res);
    return res;
}


void sem_analyse_stmt(struct ast_stmt *stmt, struct SymbolTable *st){
    if(!stmt) return;
    if(stmt->kind == STMT_DECL){
        if(stmt->decl)
            sem_analyse_decl(stmt->decl, st);
    }else if(stmt->kind == STMT_EXPR){
        if(stmt->main_expr){
            auto t = sem_analyse_expr(stmt->main_expr, st);
            ast_vtype_free(t);
        }
    }else if(stmt->kind == STMT_IFELSE){
        auto t = sem_analyse_expr(stmt->main_expr, st);
        ast_vtype_free(t);
        if(stmt->main_code) {
            sem_analyse_stmt(stmt->main_code, st);
        }
        if(stmt->else_code){
            sem_analyse_stmt(stmt->else_code, st);
        }
    }else if(stmt->kind ==  STMT_BLOCK){
        sym_table_enter(st);
        sem_analyse_stmt(stmt->main_code, st);
        sym_table_pop(st);
    }else if(stmt->kind == STMT_FOR){
        struct SymbolList *sl = create_sym_list();
        sym_table_push(st, sl);
        if(stmt->init_expr){
            auto t = sem_analyse_expr(stmt->init_expr, st);
            ast_vtype_free(t);
        }
        if(stmt->main_expr){
            auto t = sem_analyse_expr(stmt->main_expr, st);
            if(t->kind != VTYPE_BOOL){
                sem_fail_print("For loop middle expression is not a boolean.");
            }
            ast_vtype_free(t);
        }
        if(stmt->next_expr){
            auto t = sem_analyse_expr(stmt->next_expr, st);
            ast_vtype_free(t);
        }
        if(stmt->main_code) {
            sem_analyse_stmt(stmt->main_code, st);
        }
        sym_table_pop(st);
    }else if(stmt->kind == STMT_RETURN){
        //Probably find this on the symbol table itself??
        if(st->ret_vtype->kind == VTYPE_VOID){
            if(stmt->main_expr)
                sem_fail_print("Returned a value on a void function.");
        }else{
            auto ret = sem_analyse_expr(stmt->main_expr, st);
            if(!vtype_match(st->ret_vtype, ret)){
                sem_fail_print("Return doesn't match function return.");
            }
            ast_vtype_free(ret);
        }
    }else if(stmt->kind == STMT_EMPTY_BLOCK){
    }else if(stmt->kind == STMT_PRINT){
        auto t = sem_analyse_expr(stmt->main_expr,st);
        ast_vtype_free(t);
    }else{
        sem_fail_print("Somehow gave a non-existent statement kind. Nice!");
    }
    if(stmt->next_stmt)
        sem_analyse_stmt(stmt->next_stmt, st);
}


void sem_analyse_decl(struct ast_decl *decl, struct SymbolTable *st){
    /*
     * Analysis structure:
     * 1.   Get the id and search the symbol table if it exists.
     *      Only exception: global functions are allowed to be defined (once) after declaration.
     * 2.   Insert id into the symbol table with its type.
     * 3.
     *      a. If function definition, create new scope and put args on it. Check code.
     *      b. Otherwise, check that initialization matches type.
     */
    char *name = decl->id;
    int stack_lvl = st->sz - 1;
    // Phase 1. Get the id and search the symbol table if it exists.
    auto found_s = sym_table_lookup(st, decl->id);
    if(found_s){
        //Found duplicate symbol. Check if it's a global fn defn.
        if(decl->type->kind == VTYPE_FN &&
            st->sz == 1 && decl->code &&
            !found_s->is_defined
        ){
            if(!vtype_match(decl->type, found_s->type))
                sem_fail_print("Function arguments in definition don't match declaration.");
        }
        else if(found_s->scope_level == st->sz-1 ){
            sem_fail_print("Declared the same variable/function twice.\nVariable: %s", decl->id);
        }
    }else{
        stack_lvl = st->sz-1; //set to highest scope
    }
    // Phase 2. Insert id into the symbol table with its type.
    if(decl->type->kind == VTYPE_FN){
        sym_list_insert(st,
                           name,
                           stack_lvl,
                           decl->type,
                           false,
                           decl->code != 0);
    }else{
        sym_list_insert(st,
                        name,
                        stack_lvl,
                        decl->type,
                        false,
                        decl->val != 0);
    }
    // Phase 3. Statement and expression matching.
    if(decl->type->kind == VTYPE_FN && decl->code){
        //Phase 3. a.
        //Function definitions should always be global.
        if(st->sz > 1){
            sem_fail_print("Function definitions should only be global.");
        }

        struct SymbolList *sl = create_sym_list();
        sym_table_push(st, sl);
        stack_lvl++;
        //Begin function scope
        st->ret_vtype = decl->type->subtype;
        struct ast_param_list *args = decl->type->args;
        while(args){
            //Should check no duplicate names.
            char *arg_name = args->name;
            if(sym_list_find(sym_table_get(st, stack_lvl), arg_name)){
                sem_fail_print("Used same function arg name twice.");
            }
            sym_list_insert(st,
                            arg_name,
                            stack_lvl,
                            args->type,
                            true,
                            false);
            args = args->next;
        }
        sem_analyse_stmt(decl->code, st);
        st->ret_vtype = 0;
        //End function scope
        sym_table_pop(st);
    }else if(decl->type->kind != VTYPE_FN && decl->val){
        //Phase 3. b.
        //Normal type-checking.
        auto newtype = sem_analyse_expr(decl->val, st);
        if(!vtype_match(decl->type, newtype)){
            sem_fail_print("Initialization value doesn't match variable type. Types are %s and %s with sizes %i and %i",
                type_lookup[decl->type->kind], type_lookup[newtype->kind], decl->type->arr_sz, newtype->arr_sz);

        }
        sym_list_insert(st,
                        name,
                        stack_lvl,
                        decl->type,
                        false,
                        true);
   }
}

void sem_analyse(struct ast_decl *ast){
    struct SymbolTable *st = create_sym_table();
    struct SymbolList *sl = create_sym_list();
    sym_table_push(st, sl);
    while(ast){
        sem_analyse_decl(ast, st);
        ast = ast->next;
    }
    sym_table_pop(st); //cleanup
}
