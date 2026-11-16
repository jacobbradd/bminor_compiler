#include "ast.h"
#include "dag_simple.h"
#include <stdio.h>
#include <stdlib.h>
struct dag_node *_dagify_expr_helper(struct dag_node **root, struct ast_expr *expr);
struct control_flow_node *cf_gen_block(struct ast_stmt *stmt);
struct control_flow_node *cf_gen_stmt(struct ast_stmt *stmt);
struct control_flow_node *cf_gen_decl(struct ast_decl *decl);
unsigned get_type_size(struct ast_vtype *type);
struct dag_node *_check_dag_exists(struct dag_node *root, struct dag_node *new_dag);
void _insert_dag_or_replace(struct dag_node **root, struct dag_node **new_dag);


void print_dag(struct dag_node *d){
    if(!d) return;
    switch(d->kind){
        case DAG_DEREF:
            printf("DEREF(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_FN_CALL:
            printf("FN(");
            print_dag(d->left);
            if(d->right) {
                auto goer = d->right;
                while(goer){
                    printf(",");
                    print_dag(goer->left);
                    goer = goer->right;
                }
            }
            printf(")");
            break;
        case DAG_INCREMENT:
            printf("INC(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_DECREMENT:
            printf("DEC(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_NOT:
            printf("LOGNOT(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_BITNOT:
            printf("BITNOT(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_NEGATION:
            printf("NEG(");
            print_dag(d->left);
            printf(")");
            break;
        case DAG_MULT:
            printf("MUL(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_MOD:
            printf("MUL(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_DIV:
            printf("DIV(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_PLUS:
            printf("ADD(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_MINUS:
            printf("SUB(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_LESS:
            printf("LESS(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_GREATER:
            printf("MORE(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_LEQ:
            printf("LEQ(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_GEQ:
            printf("GEQ(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_EQUALITY:
            printf("EQ(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_INEQUALITY:
            printf("NEQ(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_BITAND:
            printf("BITAND(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_BITXOR:
            printf("BITXOR(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");
            break;
        case DAG_BITOR:
            printf("BITOR(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");

            break;
        case DAG_AND:
            printf("LOGAND(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");

            break;
        case DAG_OR:
            printf("LOGOR(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");

            break;
        case DAG_ASSIGN:
            printf("ASSIGN(");
            print_dag(d->left);
            printf(",");
            print_dag(d->right);
            printf(")");

            break;
        case DAG_FN_ARG:

            break;
        case DAG_NAME:
            printf("%s", d->name);
            break;
        case DAG_LIT_BOOL:
            printf("%s", d->lit_int ? "true" : "false");

            break;
        case DAG_LIT_INT:
            printf("%d", d->lit_int);


            break;
        case DAG_LIT_CHAR:
            printf("%c", d->lit_int);

            break;
        case DAG_LIT_STRING:
            printf("%s", d->lit_string);
            break;
        case DAG_ARR_MEMBER:
            break;
        default:
            break;
    }
}

//This is a glorified sizeof() operator.
unsigned get_type_size(struct ast_vtype *type){
    //returns size in.. what? words?
    switch(type->kind){
        case VTYPE_BOOL:
        case VTYPE_CHAR:
        case VTYPE_INT:
        case VTYPE_STRING:
            return 1;
        case VTYPE_ARRAY:
            return type->arr_sz * get_type_size(type->subtype);
        default:
            fprintf(stderr, "WARNING: Calling type size of VOID or FN.\n");
            return 0;
    }
}

struct dag_node *_check_dag_exists(struct dag_node *root, struct dag_node *new_dag){

    /*
     * Checks if a literal already exists. But I think I won't use this after all.
     */
    while(root)
    {
        if(root->left == new_dag->left && root->right == new_dag->right && root->kind == new_dag->kind){
            if(root->kind == DAG_LIT_BOOL || root->kind == DAG_LIT_CHAR || root->kind == DAG_LIT_INT){
                if(root->lit_int == new_dag->lit_int){
                    return root;
                }
            }else if(root->kind == DAG_LIT_STRING){
                if(root->lit_string == new_dag->lit_string){
                    return root;
                }
            }else if(root->kind == DAG_NAME){
                if(root->name == new_dag->name){
                    return root;
                }
            }else{
                return root;
            }
        }
        root = root->next;
    }
    return 0;
}

void _insert_dag_or_replace(struct dag_node **root, struct dag_node **new_dag){
    /*
     * Value numbering: look for node if already exists and reuse.
     *
     */
    auto found_d = _check_dag_exists(*root, *new_dag);
    if(found_d){
       free(*new_dag);
       *new_dag = found_d;
    }else{
        //insert: TODO this is slow; use dynamic array
        if(*root == 0){
            *root = *new_dag;
        }else{
            auto goer = *root;
            while(goer->next) goer = goer->next;
            goer->next = *new_dag;
        }
    }
}

void _dag_constant_fold(struct dag_node *dag, struct dag_node **l, struct dag_node **r){

    //extremely basic constant folding
    //NOTE that there can be dead leaves due to conflicting with val-numbering
    //NOTE for future: find a canonical ordering for commutative operations

    switch(dag->kind){
        case DAG_NOT:
            if((*l)->kind == DAG_LIT_BOOL){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = 1 - (*l)->lit_int;
                (*l)=0;
                dag->ershov = 1;
            }break;
        case DAG_BITNOT:
            if((*l)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ~((*l)->lit_int);
                (*l)=0;
                dag->ershov = 1;
            }break;
        case DAG_NEGATION:
            if((*l)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = -((*l)->lit_int);
                (*l)=0;
                dag->ershov = 1;
            }break;
        case DAG_MULT:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) * ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_MOD:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                if((*r)->lit_int == 0){
                    fprintf(stderr, "Fail: modulo by 0.\n");
                    exit(1);
                }
                dag->lit_int = ((*l)->lit_int) % ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_DIV:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                if((*r)->lit_int == 0){
                    fprintf(stderr, "Fail: division by 0.\n");
                    exit(1);
                }
                dag->lit_int = ((*l)->lit_int) / ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_PLUS:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) + ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_MINUS:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) - ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_LESS:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) < ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_GREATER:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) > ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_GEQ:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) >= ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_LEQ:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) <= ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_EQUALITY:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) == ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }if((*l)->kind == DAG_LIT_CHAR && (*r)->kind == DAG_LIT_CHAR){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) == ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_INEQUALITY:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) != ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }if((*l)->kind == DAG_LIT_CHAR && (*r)->kind == DAG_LIT_CHAR){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) != ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_BITAND:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) & ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_BITXOR:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) ^ ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_BITOR:
            if((*l)->kind == DAG_LIT_INT && (*r)->kind == DAG_LIT_INT){
                dag->kind = DAG_LIT_INT;
                dag->lit_int = ((*l)->lit_int) | ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_OR:
            if((*l)->kind == DAG_LIT_BOOL && (*r)->kind == DAG_LIT_BOOL){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) || ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        case DAG_AND:
            if((*l)->kind == DAG_LIT_BOOL && (*r)->kind == DAG_LIT_BOOL){
                dag->kind = DAG_LIT_BOOL;
                dag->lit_int = ((*l)->lit_int) && ((*r)->lit_int);
                (*l)=0;
                (*r)=0;
                dag->ershov = 1;
            }break;
        default:
    }
}


struct dag_node *_dagify_expr_arr_access_helper(struct dag_node **root, struct ast_expr *expr){
    if(expr->kind == EXPR_ARR_ACCESS && expr->left->kind == EXPR_ARR_ACCESS){
        struct dag_node* new_dag = calloc(1, sizeof(*new_dag));
        new_dag->kind = DAG_PLUS;
        new_dag->left = _dagify_expr_arr_access_helper(root, expr->left);
        new_dag->right = calloc(1, sizeof(struct dag_node));
        new_dag->right->kind = DAG_MULT;
        new_dag->right->left = _dagify_expr_helper(root, expr->right);
        new_dag->right->right = calloc(1, sizeof(struct dag_node));
        new_dag->right->right->kind = DAG_LIT_INT;
        new_dag->right->right->ershov = 1;
        new_dag->right->right->lit_int = get_type_size(expr->type);
        new_dag->right->right->type = create_ast_vtype_normal(VTYPE_INT);
        new_dag->right->type = create_ast_vtype_normal(VTYPE_INT);
        new_dag->type = create_ast_vtype_normal(VTYPE_INT);
        new_dag->right->ershov = new_dag->right->right->ershov + (new_dag->right->right->ershov == 1);
        new_dag->ershov = new_dag->right->ershov;
        if(new_dag->left->ershov >= new_dag->right->ershov) new_dag->ershov = new_dag->left->ershov + (new_dag->right->ershov == new_dag->left->ershov);
        _insert_dag_or_replace(root, &new_dag);
        return new_dag;
    }else if(expr->kind == EXPR_ARR_ACCESS && expr->left->kind == EXPR_NAME){
        struct dag_node* new_dag = calloc(1, sizeof(struct dag_node));
        new_dag->kind = DAG_MULT;
        new_dag->left = _dagify_expr_helper(root, expr->right);
        new_dag->right = calloc(1, sizeof(struct dag_node));
        new_dag->right->kind = DAG_LIT_INT;
        new_dag->right->ershov = 1;
        new_dag->right->lit_int = get_type_size(expr->type);
        new_dag->right->type = create_ast_vtype_normal(VTYPE_INT);
        new_dag->type = create_ast_vtype_normal(VTYPE_INT);
        new_dag->ershov = new_dag->right->ershov + (new_dag->right->ershov == 1);
        _insert_dag_or_replace(root, &new_dag);
        return new_dag;
    }else{
        fprintf(stderr, "Runtime error: dagify expr access helper somehow didn't hit EXPR_A_A.\n");
        __builtin_trap();
        exit(1);
    }
}

struct dag_node *_dagify_expr_arr_access(struct dag_node **root, struct ast_expr *expr){
    /*
     * TODO
     * 1. Adjust sizeof code to handle different type sizes (e.g. char is a byte) [big change]
     * 2. Create a test suite to make sure ershov is correct.
     */
    struct dag_node *new_dag  = calloc(1, sizeof(*new_dag));
    new_dag->kind = DAG_DEREF;
    auto arr_name_node = expr->left;
    while(arr_name_node->kind == EXPR_ARR_ACCESS) arr_name_node = arr_name_node->left;
    new_dag->left = _dagify_expr_helper(root, arr_name_node);
    new_dag->right = _dagify_expr_arr_access_helper(root, expr);
    new_dag->type = ast_vtype_copy(expr->left->type->subtype);
    new_dag->ershov = new_dag->right->ershov;
    _insert_dag_or_replace(root, &new_dag);
    return new_dag;
}

struct dag_node *_dagify_expr_helper(struct dag_node **root, struct ast_expr *expr){
    if(expr->kind == EXPR_ARR_ACCESS){
        return _dagify_expr_arr_access(root, expr);
    }
    struct dag_node *l = 0;
    struct dag_node *r = 0;
    if(expr->left) l =_dagify_expr_helper(root, expr->left);
    if(expr->right) r = _dagify_expr_helper(root, expr->right);


    struct dag_node *new_dag = calloc(1, sizeof(*new_dag));
    switch(expr->kind){
        case EXPR_NAME:
            new_dag->kind = DAG_NAME;
            new_dag->name = expr->name;
            new_dag->ershov = 1;
            break;
        case EXPR_LIT_BOOL:
        case EXPR_LIT_CHAR:
        case EXPR_LIT_INT:
            new_dag->kind = (dag_kind_t)expr->kind;
            new_dag->lit_int = expr->lit_int;
            new_dag->ershov = 1;
            break;
        case EXPR_LIT_STRING:
            new_dag->kind = DAG_LIT_STRING;
            new_dag->lit_string = expr->lit_string;
            new_dag->ershov = 1;
        default:
            new_dag->kind = (dag_kind_t)expr->kind;
    }
    _dag_constant_fold(new_dag, &l, &r);
    new_dag->type = ast_vtype_copy(expr->type);
    ///////////////////////////////// Ershov begin
    //TODO check I did ershov right here...
    //I don't think I did. It really just checks if you do the same thing twice
    if(l) {
        new_dag->left = l;
        new_dag->ershov = l->ershov;
    }
    if(r) {
        new_dag->right = r;
        new_dag->ershov = r->ershov;
    }
    if(new_dag->kind == DAG_FN_ARG || new_dag->kind == DAG_FN_CALL ){
        if(l) new_dag->ershov = l->ershov;
        if(r != 0 && r->ershov > new_dag->ershov) new_dag->ershov = r->ershov;
    }else if(l && r){
        if(new_dag->kind == DAG_AND || new_dag->kind == DAG_OR){
            //Due to jumping, it's always just in the first register
            new_dag->ershov = 1;
        }else if(l == r){
            new_dag->ershov = l->ershov;
        }else if(l->ershov == r->ershov) {
            new_dag->ershov = l->ershov + 1;
        }else if(l->ershov > r->ershov){
            new_dag->ershov = l->ershov;
        }else{
            new_dag->ershov = r->ershov;
        }
    }
    ///////////////////////////////// Ershov end


    _insert_dag_or_replace(root, &new_dag);
    return new_dag;
}

struct dag_node *dagify_expr(struct ast_expr *expr){
    struct dag_node *root = 0;
    return _dagify_expr_helper(&root, expr);
}


struct control_flow_node *_cf_get_tail(struct control_flow_node *root){
    if(root == 0) return 0;
    //to deal with loops, always exit via false
    while(root->next) {
        if(root->false_next) {
            root = root->false_next;
        } else {
            root = root->next;
        }
    }
    return root;

}


struct control_flow_node *cf_gen_block(struct ast_stmt *stmt){
    if(stmt->kind == STMT_EMPTY_BLOCK) return cf_create_nop();
    auto res = cf_create_scope_enter();
    auto goer = res;
    while(stmt){
        goer->next = cf_gen_stmt(stmt);
        if(goer->next) {
            if(goer->next->fn_ershov > res->fn_ershov){
                res->fn_ershov = goer->next->fn_ershov;
            }
            if(goer->next->fn_call_args > res->fn_call_args){
                res->fn_call_args = goer->next->fn_call_args;
            }
            res->fn_decls += goer->next->fn_decls;
            goer = _cf_get_tail(goer->next);
        }
        stmt = stmt->next_stmt;
    }
    goer->next = cf_create_scope_exit();
    return res;
}

struct control_flow_node *cf_gen_fn_block(struct ast_stmt *stmt){
    if(stmt->kind == STMT_EMPTY_BLOCK) return cf_create_nop();
    //initial scope is handled outside of function declaration
    //so that arguments also enter scope

    auto res = cf_gen_stmt(stmt);
    stmt = stmt->next_stmt;
    //auto res = cf_create_scope_enter();
    auto goer = res;
    while(stmt){
        goer->next = cf_gen_stmt(stmt);
        if(goer->next) {
            if(goer->next->fn_ershov > res->fn_ershov){
                res->fn_ershov = goer->next->fn_ershov;
            }
            if(goer->next->fn_call_args > res->fn_call_args){
                res->fn_call_args = goer->next->fn_call_args;
            }
            res->fn_decls += goer->next->fn_decls;
            goer = _cf_get_tail(goer->next);
        }
        stmt = stmt->next_stmt;
    }
    //goer->next = cf_create_scope_exit();
    return res;
}


struct control_flow_node *cf_gen_stmt(struct ast_stmt *stmt){
    if(stmt == 0) {
        fprintf(stderr, "ERROR: 0 Statement found during CF generation.\n");
        exit(1);
    }
    switch(stmt->kind){
        case STMT_DECL:
            return cf_gen_decl(stmt->decl);
        case STMT_EXPR:
            return cf_create_dag(stmt->main_expr);
        case STMT_BLOCK:
            return cf_gen_block(stmt->main_code);
        case STMT_RETURN:
            return cf_create_ret(stmt->main_expr);
        case STMT_IFELSE:{
            auto res = cf_create_dag(stmt->main_expr);
            res->next = cf_gen_stmt(stmt->main_code);
            if(res->next->fn_ershov > res->fn_ershov){
                res->fn_ershov = res->next->fn_ershov;
            }
            if(res->next->fn_call_args > res->fn_call_args){
                res->fn_call_args = res->next->fn_call_args;
            }
            auto main_after = _cf_get_tail(res->next);
            main_after->next = cf_create_endif();
            if(stmt->else_code){
                res->false_next = cf_gen_stmt(stmt->else_code);
                if(res->false_next->fn_ershov > res->fn_ershov){
                    res->fn_ershov = res->false_next->fn_ershov;
                }
                if(res->false_next->fn_call_args > res->fn_call_args){
                    res->fn_call_args = res->false_next->fn_call_args;
                }
                auto false_after = _cf_get_tail(res->false_next);
                false_after->next = cf_create_endelse();
                false_after->next->next = main_after->next;
            }else{
                res->false_next = cf_create_endelse();
                res->false_next->next = main_after->next;
            }
            return res;
        }
        case STMT_FOR:{
            //I allowed empty expressions in for loops, so I have to account for this

            //Apparently expressions in a for loop are scoped
            struct control_flow_node *res = cf_create_scope_enter();
            struct control_flow_node * goer = 0;
            struct control_flow_node * for_conditional = 0;
            if(stmt->main_expr){
                for_conditional = cf_create_dag(stmt->main_expr);
                if(for_conditional->fn_ershov > res->fn_ershov){
                    res->fn_ershov = for_conditional->fn_ershov;
                }
                if(for_conditional->fn_call_args > res->fn_call_args){
                    res->fn_call_args = for_conditional->fn_call_args;
                }
            }else{
                //Empty expression means infinite loop.
                auto t = create_expr_lit_true();
                for_conditional = cf_create_dag(t);
                free(t);
            }
            if(stmt->init_expr){
                res->next = cf_create_dag(stmt->init_expr);
                if(res->next->fn_ershov > res->fn_ershov){
                    res->fn_ershov = res->next->fn_ershov;
                }
                if(res->next->fn_call_args > res->fn_call_args){
                    res->fn_call_args = res->next->fn_call_args;
                }
                res->next->next = for_conditional;
            }else{
                res->next = for_conditional;
            }
            for_conditional->false_next = cf_create_endfor();
            for_conditional->false_next->next = cf_create_scope_exit();
            //HACK fix this garbage
            for_conditional->next = cf_gen_stmt(stmt->main_code);
            if(for_conditional->next->fn_ershov > res->fn_ershov){
                res->fn_ershov = for_conditional->next->fn_ershov;
            }
            if(for_conditional->next->fn_call_args > res->fn_call_args){
                res->fn_call_args = for_conditional->next->fn_call_args;
            }
            goer = _cf_get_tail(for_conditional->next);
            if(stmt->next_expr){
                goer->next = cf_create_dag(stmt->next_expr);
                if(goer->next->fn_ershov > res->fn_ershov){
                    res->fn_ershov = goer->next->fn_ershov;
                }
                if(goer->next->fn_call_args > res->fn_call_args){
                    res->fn_call_args = goer->next->fn_call_args;
                }
                goer = goer->next;
            }
            goer->next = cf_create_goto();
            goer->next->next = for_conditional;
            return res;
        }
        case STMT_EMPTY_BLOCK:
            return cf_create_nop();
        case STMT_PRINT:
            return cf_create_print(stmt->main_expr);
        default:
            return 0;
    }

}

struct control_flow_node *cf_gen_decl(struct ast_decl *decl){
    if(decl->type->kind == VTYPE_FN && decl->code){
        struct control_flow_node *res = cf_create_decl(decl);
        res->code = cf_gen_fn_block(decl->code);
        res->fn_ershov = res->code->fn_ershov;
        res->fn_call_args = res->code->fn_call_args;
        res->fn_decls = res->code->fn_decls;
        return res;
    }else if(decl->type->kind != VTYPE_FN){
        return cf_create_decl(decl);
    }else{
        return 0;
    }
}

struct control_flow_node * cf_gen(struct ast_decl *program){
    struct control_flow_node *res = 0;
    struct control_flow_node *goer = 0;
    while(program){
        if(res == 0){
            res = cf_gen_decl(program);
            goer = _cf_get_tail(res);
        }else{
            goer->next = cf_gen_decl(program);
            if(goer->next) goer = _cf_get_tail(goer->next);
        }
        program = program->next;
    }

    return res;
}







////////////////


struct control_flow_node *cf_create_decl(struct ast_decl* decl){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_DECL;
    res->decl = decl;
    if(decl->val) {
        res->dag = dagify_expr(decl->val);
        res->fn_ershov = res->dag->ershov;
        if(decl->type->kind != VTYPE_ARRAY){
            res->fn_decls = 1;
        }else{
            res->fn_decls = decl->type->arr_sz * decl->type->arr_stride;
        }
    }
    return res;
}
struct control_flow_node *cf_create_nop(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_NOP;
    return res;
}
struct control_flow_node *cf_create_endfor(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_ENDFOR;
    return res;
}
struct control_flow_node *cf_create_endif(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_ENDIF;
    return res;
}
struct control_flow_node *cf_create_dag(struct ast_expr* expr){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_DAG;
    res->dag = dagify_expr(expr);
    res->fn_call_args = 0;
    //TODO this isn't nearly enough
    if(res->dag->kind == DAG_FN_CALL){
        auto goer = res->dag->right;
        while(goer){
            res->fn_call_args++;
            goer = goer->right;
        }
    }
    res->fn_ershov = res->dag->ershov;
    return res;

}
struct control_flow_node *cf_create_ret(struct ast_expr* expr){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_RET;
    res->dag = dagify_expr(expr);
    res->fn_ershov = res->dag->ershov;
    if(res->dag->kind == DAG_FN_CALL){
        auto goer = res->dag->right;
        while(goer){
            res->fn_call_args++;
            goer = goer->right;
        }
    }
    return res;

}
struct control_flow_node *cf_create_goto(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_GOTO;
    return res;
}
struct control_flow_node *cf_create_scope_enter(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_SCOPE_ENTER;
    return res;

}
struct control_flow_node *cf_create_scope_exit(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_SCOPE_EXIT;
    return res;
}
struct control_flow_node *cf_create_print(struct ast_expr* expr){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_PRINT;
    res->dag = dagify_expr(expr);
    res->fn_ershov = res->dag->ershov;
    if(res->dag->kind == DAG_FN_CALL){
        auto goer = res->dag->right;
        while(goer){
            res->fn_call_args++;
            goer = goer->right;
        }
    }
    return res;
}

struct control_flow_node *cf_create_endelse(){
    struct control_flow_node *res = calloc(1, sizeof(*res));
    res->kind = CF_ENDELSE;
    return res;
}




