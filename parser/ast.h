/*
 * ast.h
 *
 * An AST structure for B-minor as described in the lecture notes book.
 *
 * Defines structs for the program AST. We can define a program as a (linked) list of declarations.
 * A declaration consists of an id (name) and type (to make a variable), and optionally an expression, or a statement (list) if assigned to a function.
 *
 * A statement can consist of many things, but can be a a declaration, an expression, a code block of statements, an if/else code blocks, a for block, etc.
 *
 * An expression can be a function call, a return, arithmetic/logical expressions, literals or variables, assignments, etc.
 * An expression is just a binary AST, which at its core consists of a value (this is the usual expression AST).
 *
 * Note that `for` consists of 3 expressions with a code block, and `if/else` consists of 1 expression with up to 2 code blocks.
 * Therefore, a "statement" can consist of a declaration, up to 3 expressions, and up to 2 statements, as well as pointing to the next statement.
 *
 * A type consists of the type itself (as an enum), and a potential subttype for arrays, and a parameter list for functions.
 * Function calling: will express as a tree of arguments (like a linked list but worse).
 */

#ifndef __AST_H
#define __AST_H

typedef enum {
    VTYPE_BOOL,
    VTYPE_INT,
    VTYPE_CHAR,
    VTYPE_STRING,
    VTYPE_ARRAY,
    VTYPE_FN,
    VTYPE_VOID
} vtype_kind_t;

typedef enum {
    STMT_DECL,
    STMT_EXPR,
    STMT_IFELSE,
    STMT_BLOCK,
    STMT_FOR,
    STMT_RETURN,
    STMT_EMPTY_BLOCK,
    STMT_PRINT
} stmt_kind_t;

typedef enum { //ordered by high-to-low operator precedence
    EXPR_ARR_ACCESS,
    EXPR_FN_CALL,
    EXPR_INCREMENT,
    EXPR_DECREMENT,
    EXPR_NOT,
    EXPR_BITNOT,
    EXPR_NEGATION,
    EXPR_MULT,
    EXPR_MOD,
    EXPR_DIV,
    EXPR_PLUS,
    EXPR_MINUS,
    EXPR_LESS,
    EXPR_GREATER,
    EXPR_LEQ,
    EXPR_GEQ,
    EXPR_EQUALITY,
    EXPR_INEQUALITY,
    EXPR_BITAND,
    EXPR_BITXOR,
    EXPR_BITOR,
    EXPR_AND,
    EXPR_OR,
    EXPR_ASSIGN,
    EXPR_FN_ARG,
    EXPR_NAME, //this and below are "terminals"
    EXPR_LIT_BOOL,
    EXPR_LIT_INT,
    EXPR_LIT_CHAR,
    EXPR_LIT_STRING,
    EXPR_ARR_MEMBER,
} expr_kind_t;


struct ast_decl{
    char *id;
    struct ast_vtype *type;
    struct ast_expr *val;
    struct ast_stmt *code;
    struct ast_decl *next;
};

struct ast_vtype{
    vtype_kind_t kind;
    unsigned arr_sz; //for arrays
    unsigned arr_stride;
    struct ast_vtype *subtype;
    struct ast_param_list *args;
};

struct ast_param_list{ //a linked list of variable prototypes
    char *name;
    struct ast_vtype *type;
    struct ast_param_list *next;
};

struct ast_stmt{
    stmt_kind_t kind;
    struct ast_decl *decl;
    struct ast_expr *init_expr; //for for loops
    struct ast_expr *main_expr;
    struct ast_expr *next_expr; //for for loops
    struct ast_stmt *main_code;
    struct ast_stmt *else_code;
    struct ast_stmt *next_stmt; //linked list
};


//it might be a good idea to add a vtype_kind_t to be populated by the typechecker
struct ast_expr{
    expr_kind_t kind; //the "value", or kind if name, integer, char or string
    struct ast_expr *left;
    struct ast_expr *right;
    struct ast_vtype *type; //the "type" of the expression: filled by sem_analyser
    //int ershov; //new skill acquired: Ershov number
    union{
        char * name; //if name
        int lit_int; //if an integer or char
        char * lit_string; //if a string
    };
};

struct ast_decl *create_ast_decl(
    char *id,
    struct ast_vtype *type,
    struct ast_expr *val,
    struct ast_stmt *code,
    struct ast_decl *next
);

struct ast_vtype *create_ast_vtype_normal(vtype_kind_t kind);
struct ast_vtype *create_ast_vtype_array(unsigned arr_sz, struct ast_vtype *subtype);
struct ast_vtype *create_ast_vtype_fn(struct ast_vtype *ret_vtype, struct ast_param_list *args);

struct ast_param_list *create_param_arg(
    char *name,
    struct ast_vtype *type
);

struct ast_stmt *create_stmt_decl(struct ast_decl *decl);
struct ast_stmt *create_stmt_for(
    struct ast_expr *init_expr,
    struct ast_expr *main_expr,
    struct ast_expr *next_expr,
    struct ast_stmt *main_code
);

struct ast_stmt *create_stmt_ifelse(
    struct ast_expr *main_expr,
    struct ast_stmt *main_code,
    struct ast_stmt *else_code
);

struct ast_stmt *create_stmt_expr(struct ast_expr *main_expr);
struct ast_stmt *create_stmt_blck(struct ast_stmt *main_code);

struct ast_expr *create_expr_op(expr_kind_t kind, struct ast_expr *left, struct ast_expr *right); //for typical operators
struct ast_expr *create_expr_name(char *name);
struct ast_expr *create_expr_lit_int(int lit);
struct ast_expr *create_expr_lit_char(char lit);
struct ast_expr *create_expr_lit_true();
struct ast_expr *create_expr_lit_false();
struct ast_expr *create_expr_lit_string(char *string);
struct ast_expr *create_expr_arr_access(struct ast_expr * acc_ind, struct ast_expr *arr);
struct ast_expr *create_expr_fn_call(struct ast_expr *name, struct ast_expr *args);
struct ast_expr *create_expr_fn_arg(struct ast_expr *arg_val, struct ast_expr *next_args);
struct ast_expr *create_expr_arr_member(struct ast_expr *member);
struct ast_stmt *create_stmt_return(struct ast_expr *main_expr);
struct ast_stmt *create_stmt_empty_block();
struct ast_stmt *create_stmt_print(struct ast_expr *main_expr);
struct ast_vtype * ast_vtype_copy(struct ast_vtype *type);
void ast_vtype_free(struct ast_vtype *type);
bool vtype_match(struct ast_vtype *type1, struct ast_vtype *type2);

#endif

