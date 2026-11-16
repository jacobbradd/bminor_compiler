/*
 * The DAG is just like the AST for expressions, except there are some extra dag node kinds.
 * Also I'll label them closer to assembly instructions?'
 *
 */

#ifndef __DAG_SIMPLE_H
#define __DAG_SIMPLE_H

#include "ast.h"

typedef enum { //ordered by high-to-low operator precedence
    DAG_DEREF,
    DAG_FN_CALL,
    DAG_INCREMENT,
    DAG_DECREMENT,
    DAG_NOT,
    DAG_BITNOT,
    DAG_NEGATION,
    DAG_MULT,
    DAG_MOD,
    DAG_DIV,
    DAG_PLUS,
    DAG_MINUS,
    DAG_LESS,
    DAG_GREATER,
    DAG_LEQ,
    DAG_GEQ,
    DAG_EQUALITY,
    DAG_INEQUALITY,
    DAG_BITAND,
    DAG_BITXOR,
    DAG_BITOR,
    DAG_AND,
    DAG_OR,
    DAG_ASSIGN,
    DAG_FN_ARG,
    DAG_NAME,
    DAG_LIT_BOOL,
    DAG_LIT_INT,
    DAG_LIT_CHAR,
    DAG_LIT_STRING,
    DAG_ARR_MEMBER
} dag_kind_t;

struct dag_node{
    dag_kind_t kind;
    struct dag_node *left;
    struct dag_node *right;
    struct dag_node *next;
    struct ast_vtype *type;
    int ershov;
    union{
        char * name; //if name
        int lit_int; //if an integer or char
        char * lit_string; //if a string
    };
};

typedef enum {
    CF_DECL,
    CF_NOP,
    CF_RET,
    CF_DAG,
    CF_ENDIF, //to help codegen
    CF_ENDELSE, //to help codegen
    CF_ENDFOR, //to help codegen
    CF_GOTO, //to help codegen
    CF_SCOPE_ENTER,
    CF_SCOPE_EXIT,
    CF_PRINT
} cf_kind_t;


//This is a (possibly cyclic) graph representing control flow
struct control_flow_node{
    cf_kind_t kind;
    struct ast_decl* decl;
    struct dag_node *dag;
    struct control_flow_node *code; //for functions
    struct control_flow_node *next;
    struct control_flow_node *false_next; //for conditionals
    unsigned fn_ershov; //Function profiling
    unsigned fn_decls;
    unsigned fn_call_args;//max number of args in all fn calls inside the function
};


struct control_flow_node *cf_create_decl(struct ast_decl* decl);
struct control_flow_node *cf_create_nop();
struct control_flow_node *cf_create_endfor();
struct control_flow_node *cf_create_endif();
struct control_flow_node *cf_create_dag(struct ast_expr* expr);
struct control_flow_node *cf_create_ret(struct ast_expr* expr);
struct control_flow_node *cf_create_goto();
struct control_flow_node *cf_create_scope_enter();
struct control_flow_node *cf_create_scope_exit();
struct control_flow_node *cf_create_print(struct ast_expr* expr);
struct control_flow_node *cf_create_endelse();

struct dag_node *dagify_expr(struct ast_expr *expr);
struct control_flow_node * cf_gen(struct ast_decl *program);

void print_dag(struct dag_node *d);
#endif
