/*
 * ast.c
 *
 *
 *
 *
 *
 */

#include "ast.h"
#include <stdlib.h>


struct ast_decl *create_ast_decl(
    char *id,
    struct ast_vtype *type,
    struct ast_expr *val,
    struct ast_stmt *code,
    struct ast_decl *next
){
    struct ast_decl *res = calloc(1, sizeof(struct ast_decl));
    res->id = id;
    res->type = type;
    res->val = val;
    res->code = code;
    res->next = next;
    return res;
}

struct ast_vtype *create_ast_vtype_normal(vtype_kind_t kind){
    struct ast_vtype *res = calloc(1, sizeof(struct ast_vtype));
    res->kind = kind;
    return res;
}

struct ast_vtype *create_ast_vtype_array(unsigned arr_sz, struct ast_vtype *subtype){
    struct ast_vtype *res = calloc(1, sizeof(struct ast_vtype));
    res->kind = VTYPE_ARRAY;
    res->arr_sz = arr_sz;
    res->subtype = subtype;
    if(res->subtype->kind == VTYPE_ARRAY){
        res->arr_stride = res->subtype->arr_sz * res->subtype->arr_stride;
    }else{
        res->arr_stride = 1;
    }
    return res;
}

struct ast_vtype *create_ast_vtype_fn(struct ast_vtype *ret_vtype, struct ast_param_list *args){
    struct ast_vtype *res = calloc(1, sizeof(struct ast_vtype));
    res->kind = VTYPE_FN;
    res->subtype = ret_vtype;
    res->args = args;
    return res;
}

struct ast_param_list *create_param_arg(
    char *name,
    struct ast_vtype *type
){
    struct ast_param_list *res = calloc(1, sizeof(struct ast_param_list));
    res->name = name;
    res->type = type;
    return res;
}

struct ast_stmt *create_stmt_decl(struct ast_decl *decl){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_DECL;
    res->decl = decl;
    return res;
}

struct ast_stmt *create_stmt_for(
    struct ast_expr *init_expr,
    struct ast_expr *main_expr,
    struct ast_expr *next_expr,
    struct ast_stmt *main_code
){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_FOR;
    res->init_expr = init_expr;
    res->main_expr = main_expr;
    res->next_expr = next_expr;
    res->main_code = main_code;
    return res;
}

struct ast_stmt *create_stmt_ifelse(
    struct ast_expr *main_expr,
    struct ast_stmt *main_code,
    struct ast_stmt *else_code
){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_IFELSE;
    res->main_expr = main_expr;
    res->main_code = main_code;
    res->else_code = else_code;
    return res;

}

struct ast_stmt *create_stmt_expr(struct ast_expr *main_expr){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_EXPR;
    res->main_expr = main_expr;
    return res;

}
struct ast_stmt *create_stmt_blck(struct ast_stmt *main_code){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_BLOCK;
    res->main_code = main_code;
    return res;

}

struct ast_expr *create_expr_op(expr_kind_t kind, struct ast_expr *left, struct ast_expr *right){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = kind;
    res->left = left;
    res->right = right;
    //int e_max = 1;
    //if(left && left->ershov > e_max) e_max = left->ershov;
    //if(right && right->ershov > e_max) e_max = right->ershov;
    //if(left && right && left->ershov == right->ershov) e_max = left->ershov + 1;
    //res->ershov = e_max;
    return res;
}

struct ast_expr *create_expr_name(char *name){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_NAME;
    res->name = name;
    //res->ershov = 0; //leaf node; register already exists for it
    return res;
}

struct ast_expr *create_expr_lit_int(int lit){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_LIT_INT;
    res->lit_int = lit;
    //res->ershov = 0; //leaf node
    return res;
}

struct ast_expr *create_expr_lit_char(char lit){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_LIT_CHAR;
    res->lit_int = lit;
    //res->ershov = 0; //leaf node
    return res;
}


struct ast_expr *create_expr_lit_true(){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_LIT_BOOL;
    res->lit_int = 1;
    //res->ershov = 0; //leaf node
    return res;
}
struct ast_expr *create_expr_lit_false(){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_LIT_BOOL;
    res->lit_int = 0;
    //res->ershov = 0; //leaf node
    return res;
}


struct ast_expr *create_expr_lit_string(char *string){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_LIT_STRING;
    res->lit_string = string;
    return res;
}


struct ast_expr *create_expr_arr_access(struct ast_expr * acc_ind, struct ast_expr *arr){

    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_ARR_ACCESS;
    res->left = arr;
    res->right = acc_ind;
    return res;
}


struct ast_expr *create_expr_fn_call(struct ast_expr *name, struct ast_expr *args){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_FN_CALL;
    res->left = name;
    res->right = args;
    //a function call returns something, hence counts as a "leaf"
    //however, it requires a register to store the result
    //res->ershov = 1;
    return res;
}

struct ast_expr *create_expr_fn_arg(struct ast_expr *arg_val, struct ast_expr *next_args){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_FN_ARG;
    res->left = arg_val;
    res->right = next_args;
    return res;
}


struct ast_expr *create_expr_arr_member(struct ast_expr *member){
    struct ast_expr *res = calloc(1, sizeof(struct ast_expr));
    res->kind = EXPR_ARR_MEMBER;
    res->left = member;
    //res->ershov = member->ershov;
    return res;
}

struct ast_stmt *create_stmt_return(struct ast_expr *main_expr){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_RETURN;
    res->main_expr = main_expr;
    return res;

}
struct ast_stmt *create_stmt_print(struct ast_expr *main_expr){
    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_PRINT;
    res->main_expr = main_expr;
    return res;
}


struct ast_stmt *create_stmt_empty_block(){

    struct ast_stmt *res = calloc(1, sizeof(struct ast_stmt));
    res->kind = STMT_EMPTY_BLOCK;
    return res;

}


struct ast_vtype * ast_vtype_copy(struct ast_vtype *type){
    if(!type) return 0;
    auto res = create_ast_vtype_normal(type->kind);
    if(type->kind == VTYPE_ARRAY){
        res->arr_sz = type->arr_sz;
        res->arr_stride = type->arr_stride;
        res->subtype = ast_vtype_copy(type->subtype);
    }else if(type->kind == VTYPE_FN){
        res->subtype = ast_vtype_copy(type->subtype);
    }
    if(type->kind == VTYPE_FN && type->args){
        auto copied_args = create_param_arg(type->args->name, ast_vtype_copy(type->args->type));
        auto goer = type->args->next;
        res->args = copied_args;
        while(goer){
            copied_args->next = create_param_arg(goer->name, ast_vtype_copy(goer->type));
            copied_args = copied_args->next;
            goer = goer->next;
        }
    }
    return res;
}

void ast_vtype_free(struct ast_vtype *type){
    if(!type) return;
    if(type->kind == VTYPE_FN && type->args){
        while(type->args){
            auto t = type->args;
            type->args = type->args->next;
            ast_vtype_free(t->type);
        }
    }else if(type->subtype){
        ast_vtype_free(type->subtype);
    }
    free(type);
}


bool vtype_match(struct ast_vtype *type1, struct ast_vtype *type2){
    if((type1 == 0) != (type2 == 0))
        return false;
    if(type1 == 0 && type2 == 0)
        return true;
    if(type1->kind != type2->kind)
        return false;
    if(!vtype_match(type1->subtype, type2->subtype))
        return false;
    if(type1->kind == VTYPE_FN){
        //Now check the ast_param_lists
        auto pl1 = type1->args;
        auto pl2 = type2->args;
        while(pl1 && pl2){
            if(!vtype_match(pl1->type, pl2->type))
                return false;
            pl1 = pl1->next;
            pl2 = pl2->next;
        }
        if(pl1 || pl2) return false;
    }

    //TODO this has inconsistent results
    //e.g. a "camel" of nested arrays will fail this due to the fact that I check consistency
    //C doesn't even do this kind of typechecking
    //For now, the comparison is no longer < but now !=
    if(type1->kind == VTYPE_ARRAY && type1->arr_sz != type2->arr_sz) return false;
    return true;
}


