/*
 *
 * parser.c
 * The goal is to "flatten" the recursive chain by loops, thanks to Pratt/shunting/OP
 * Or really just to deal with expressions, or anything with left recursion.
 *
 * You at least need different parselets for each type: declaration, expression, statement, type.
 *
 * I'll assume the lexed output has the form (without the brackets):
 * [token number] [text_size] [text]
 * where [token number] is written always of the form e.g. 015
 * and [text_size] is written always of the form e.g. 1234567
 * So we assume you will write tokens with less than 10000000 characters lol.
 * So don't be writing your entire novel in one string please.
 *
 *
 * TODO define an auto keyword: it expects an assignment immediately (requires sem_analyser integration)
 *
 * TODO having commas in a declaration?
 * e.g.
 * a,b :int = 1 means assign 1 to a
 * a,b :int = 1, 2 means assign 1 to a, 2 to b
 * a,b :int = , 2 means assign 2 to b
 * might be too clunky.
 */


#include "ast.h"
#include "parser.h"
#include "lexer/tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static FILE * stream;
static struct Token *input;

#define TOKEN_LBRACK_PREC 200
#define TOKEN_LPAREN_PREC 200
#define TOKEN_INCREMENT_PREC 200
#define TOKEN_DECREMENT_PREC 200
#define TOKEN_LOGNOT_PREC 180
#define TOKEN_BITNOT_PREC 180
#define TOKEN_MULTIPLY_PREC 150
#define TOKEN_DIVIDE_PREC 150
#define TOKEN_MOD_PREC 150
#define TOKEN_PLUS_PREC 130
#define TOKEN_MINUS_PREC 130
#define TOKEN_LESS_PREC 90
#define TOKEN_LEQ_PREC 90
#define TOKEN_GREATER_PREC 90
#define TOKEN_GEQ_PREC 90
#define TOKEN_EQUALITY_PREC 70
#define TOKEN_INEQUALITY_PREC 70
#define TOKEN_BITAND_PREC 60
#define TOKEN_BITXOR_PREC 55
#define TOKEN_BITOR_PREC 50
#define TOKEN_LOGAND_PREC 40
#define TOKEN_LOGOR_PREC 30
#define TOKEN_EQUALS_PREC 20
#define TOKEN_ID_PREC 0
#define TOKEN_LBRACE_PREC 0
#define TOKEN_RBRACE_PREC 0
#define TOKEN_RBRACK_PREC 0
#define TOKEN_RPAREN_PREC 0
#define TOKEN_COLON_PREC 0
#define TOKEN_COMMA_PREC 0
#define TOKEN_SEMICOLON_PREC 0
#define TOKEN_EOF_PREC 0
#define TOKEN_KW_FOR_PREC 0
#define TOKEN_KW_IF_PREC 0
#define TOKEN_KW_ELSE_PREC 0
#define TOKEN_KW_BOOL_PREC 0
#define TOKEN_KW_ARRAY_PREC 0
#define TOKEN_KW_FUNCTION_PREC 0
#define TOKEN_KW_INT_PREC 0
#define TOKEN_KW_CHAR_PREC 0
#define TOKEN_KW_STRING_PREC 0
#define TOKEN_KW_RETURN_PREC 0
#define TOKEN_KW_PRINT_PREC 0
#define TOKEN_KW_VOID_PREC 0
#define TOKEN_KW_WHILE_PREC 0
#define TOKEN_LIT_TRUE_PREC 0
#define TOKEN_LIT_FALSE_PREC 0
#define TOKEN_LIT_INT_PREC 0
#define TOKEN_LIT_STRING_PREC 0
#define TOKEN_LIT_CHAR_PREC 0

static const int expr_lbp[] = {
    TOKEN_EOF_PREC,
    TOKEN_KW_FOR_PREC,
    TOKEN_KW_IF_PREC,
    TOKEN_KW_ELSE_PREC,
    TOKEN_KW_BOOL_PREC,
    TOKEN_KW_ARRAY_PREC,
    TOKEN_KW_FUNCTION_PREC,
    TOKEN_KW_INT_PREC,
    TOKEN_KW_CHAR_PREC,
    TOKEN_KW_STRING_PREC,
    TOKEN_KW_RETURN_PREC,
    TOKEN_KW_PRINT_PREC,
    TOKEN_KW_VOID_PREC,
    TOKEN_KW_WHILE_PREC,
    TOKEN_LIT_TRUE_PREC,
    TOKEN_LIT_FALSE_PREC,
    TOKEN_LIT_INT_PREC,
    TOKEN_LIT_STRING_PREC,
    TOKEN_LIT_CHAR_PREC,
    TOKEN_PLUS_PREC,
    TOKEN_MINUS_PREC,
    TOKEN_MULTIPLY_PREC,
    TOKEN_DIVIDE_PREC,
    TOKEN_MOD_PREC,
    TOKEN_LESS_PREC,
    TOKEN_LEQ_PREC,
    TOKEN_GREATER_PREC,
    TOKEN_GEQ_PREC,
    TOKEN_BITAND_PREC,
    TOKEN_BITXOR_PREC,
    TOKEN_BITOR_PREC,
    TOKEN_BITNOT_PREC,
    TOKEN_LOGAND_PREC,
    TOKEN_LOGOR_PREC,
    TOKEN_LOGNOT_PREC,
    TOKEN_INCREMENT_PREC,
    TOKEN_DECREMENT_PREC,
    TOKEN_ID_PREC,
    TOKEN_LBRACE_PREC,
    TOKEN_RBRACE_PREC,
    TOKEN_LBRACK_PREC,
    TOKEN_RBRACK_PREC,
    TOKEN_LPAREN_PREC,
    TOKEN_RPAREN_PREC,
    TOKEN_COLON_PREC,
    TOKEN_COMMA_PREC,
    TOKEN_EQUALS_PREC,
    TOKEN_SEMICOLON_PREC,
    TOKEN_EQUALITY_PREC,
    TOKEN_INEQUALITY_PREC
};

void advance_token();
void parser_fail_print(const char * err, ...);
bool is_literal(lex_token_t l);


lex_token_t peek_token(){
    return input->token;
}

lex_token_t  peek_next_token(){
    if(input->next == 0){
        advance_token();
        input = input->prev;
    }
    return input->next->token;
}

void expect_token(lex_token_t token){
    if(peek_token() != token){
        parser_fail_print("Expected %s but got %s.\n", token_to_string[token], token_to_string[peek_token()] );
    }
}

struct Token *consume_token(lex_token_t token){
    expect_token(token);
    struct Token *res = input;
    advance_token();
    return res;
}

/*
 * PRATT PARSER START
 * See notes (to be edited)
 */


struct ast_expr *expr_nud(lex_token_t cur_token){
    //Handle prefix operators.
    if(cur_token == TOKEN_MINUS){
        advance_token();
        return create_expr_op(EXPR_NEGATION, parse_expression(expr_lbp[cur_token]-1), 0);
    }
    else if(cur_token == TOKEN_LOGNOT){
        advance_token();
        return create_expr_op(EXPR_NOT, parse_expression(expr_lbp[cur_token]-1),0);
    }else if(cur_token == TOKEN_BITNOT){
        advance_token();
        return create_expr_op(EXPR_BITNOT, parse_expression(expr_lbp[cur_token]-1),0);
    }else if(cur_token == TOKEN_LPAREN){
        advance_token();
        auto res = parse_expression(0);
        consume_token(TOKEN_RPAREN);
        return res;
    }else if(cur_token == TOKEN_LBRACE){
        //This is to allow nested array literals.
        advance_token();
        bool first_run = true;
        struct ast_expr *arr_list = 0;
        struct ast_expr *goer = 0;
        while(peek_token() != TOKEN_RBRACE){
            while(first_run || peek_token() == TOKEN_COMMA){
                if(!first_run) advance_token(); //consume COMMA
                auto a_member = create_expr_arr_member(parse_expression(0));
                if(peek_token() != TOKEN_COMMA && peek_token() != TOKEN_RBRACE){
                    parser_fail_print("In array init, failed to continue listing members.\n");
                }
                if(goer){
                    goer->right = a_member;
                    goer = goer->right;
                }else{
                    arr_list = a_member;
                    goer = a_member;
                }
                first_run = false;
            }
        }
        advance_token(); //consume RBRACE
        return arr_list;


    }{ //nonud
        parser_fail_print("Reached an unexpected ender %s in an expression. Don't do empty expressions.\n", token_to_string[cur_token]);
    }

    return 0;

}

struct ast_expr *expr_led(lex_token_t cur_token, struct ast_expr *left){
    //All left-associative operators can rely on expr_lbp and be grouped together.
    //Assignment is right-assocative, the rest are left-associative.
    if(cur_token == TOKEN_PLUS){
        advance_token();
        return create_expr_op(EXPR_PLUS, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_MULTIPLY){
        advance_token();
        return create_expr_op(EXPR_MULT, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_MINUS){
        advance_token();
        return create_expr_op(EXPR_MINUS, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_DIVIDE){
        advance_token();
        return create_expr_op(EXPR_DIV, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_MOD){
        advance_token();
        return create_expr_op(EXPR_MOD, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_BITAND){
        advance_token();
        return create_expr_op(EXPR_BITAND, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_BITXOR){
        advance_token();
        return create_expr_op(EXPR_BITXOR, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_BITOR){
        advance_token();
        return create_expr_op(EXPR_BITOR, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_LOGAND){
        advance_token();
        return create_expr_op(EXPR_AND, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_LOGOR){
        advance_token();
        return create_expr_op(EXPR_OR, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_LESS){
        advance_token();
        return create_expr_op(EXPR_LESS, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_LEQ){
        advance_token();
        return create_expr_op(EXPR_LEQ, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_GREATER){
        advance_token();
        return create_expr_op(EXPR_GREATER, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_GEQ){
        advance_token();
        return create_expr_op(EXPR_GEQ, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_EQUALITY){
        advance_token();
        return create_expr_op(EXPR_EQUALITY, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_INEQUALITY){
        advance_token();
        return create_expr_op(EXPR_INEQUALITY, left, parse_expression(expr_lbp[cur_token]));
    }else if(cur_token == TOKEN_EQUALS){
        advance_token();
        return create_expr_op(EXPR_ASSIGN, left, parse_expression(expr_lbp[cur_token]-1));
    }else if(cur_token == TOKEN_INCREMENT){
        advance_token();
        return create_expr_op(EXPR_INCREMENT, left,0);
    }else if(cur_token == TOKEN_DECREMENT){
        advance_token();
        return create_expr_op(EXPR_DECREMENT, left,0);
    }else if(cur_token == TOKEN_LPAREN && left->kind == EXPR_NAME){
        //function call
        advance_token();
        bool first_run = true;
        struct ast_expr *arg_list = 0;
        struct ast_expr *goer = 0;
        while(peek_token() != TOKEN_RPAREN){
            while(first_run || peek_token() == TOKEN_COMMA){
                if(!first_run) advance_token(); //consume COMMA
                auto a_member = create_expr_fn_arg(parse_expression(0), 0);
                if(peek_token() != TOKEN_COMMA && peek_token() != TOKEN_RPAREN){
                    parser_fail_print("In function call, failed to continue listing arguments.\n");

                }
                if(goer){
                    goer->right = a_member;
                    goer = goer->right;
                }else{
                    arg_list = a_member;
                    goer = arg_list;
                }
                first_run = false;
            }
        }
        advance_token(); //consume RPAREN
        return create_expr_fn_call(left, arg_list);
    }else if(cur_token == TOKEN_LBRACK && (left->kind == EXPR_NAME || left->kind == EXPR_ARR_ACCESS)){
        //array access
        advance_token();
        auto arr_acc = parse_expression(0);
        consume_token(TOKEN_RBRACK);
        return create_expr_arr_access(arr_acc, left);
    }else{ //noled
        parser_fail_print("Operator %s not yet supported.\n", token_to_string[cur_token]);
    }
    return 0;
}

struct ast_expr *parse_expression(int right_binding_power){
    //Of course, this is best done with a Pratt parser.
    //ast_expr is a sort of "closed system": it's a pure tree of ast_expr nodes.
    //Similarly, you never leave the expression parser until you're done.
    //Note: empty expressions are (automatically) not allowed.
    struct ast_expr *left = 0;
    auto cur_token = peek_token();
    if(cur_token == TOKEN_LIT_INT){
        left = create_expr_lit_int(atoi(input->text));
        advance_token();
    }else if(cur_token == TOKEN_LIT_CHAR){
        left = create_expr_lit_char(input->text[0]);
        advance_token();
    }else if(cur_token == TOKEN_LIT_STRING){
        left = create_expr_lit_string(input->text);
        advance_token();
    }else if(cur_token == TOKEN_LIT_TRUE){
        left = create_expr_lit_true();
        advance_token();
    }else if(cur_token == TOKEN_LIT_FALSE){
        left = create_expr_lit_false();
        advance_token();
    }else if(cur_token == TOKEN_ID){
        left = create_expr_name(input->text);
        advance_token();
    }else{
        left = expr_nud(cur_token);
    }

    while(right_binding_power < expr_lbp[peek_token()]){
        lex_token_t t = peek_token();
        left = expr_led(t, left);
    }
    return left;
}

/*
 * PRATT PARSER END
 *
 */














void advance_token(){
    if(input && input->next){
        input = input->next;
    }else{
        struct Token *next_token = calloc(1, sizeof(struct Token));
        unsigned sz;
        int val_intermediate = 0;
        lex_token_t val = 0;
        if(fscanf(stream, " %03d %07d ",&val_intermediate, &sz) != 2){
            printf("Another reading fail.\n");
            exit(1);
        }
        val = (lex_token_t) val_intermediate;
        next_token->token = val;

        //Handle strings and chars (unclip them)
        if(val == TOKEN_LIT_STRING || val == TOKEN_LIT_CHAR){
            sz -= 2;
            fgetc(stream);
        }
        char *next_line = calloc(sz+1,sizeof(char));
        unsigned num_chars_read = fread(next_line, 1,sz, stream);
        if(sz != num_chars_read){
            printf("Reading fail. That's my bad.\n");
            exit(1);
        }
        //Handle strings and chars (unclip them)
        if(val == TOKEN_LIT_STRING || val == TOKEN_LIT_CHAR){
            fgetc(stream);
        }
        while(fgetc(stream) != '\n'){} //newline

        next_token->text = next_line;
        if(input){
            input->next = next_token;
            next_token->prev = input;
            input = next_token;
        }else{
            input = next_token;
        }
#ifdef DEBUG
        printf("Advance: %s with text %s\n", token_to_string[input->token], input->text);
#endif
    }
}

void parser_fail_print(const char * err, ...){
    va_list args;
    va_start(args, err);
    fprintf(stderr, "Parser fail: ");
    vfprintf(stderr, err, args);
    fprintf(stderr, "Last 10 tokens:\n");
    auto old_input = input;
    for(int i = 0; i < 11 && input != 0 && input->prev != 0; i++){
        input = input->prev;
    }
    while(input != old_input){
        fprintf(stderr, "Token: %s. Text: %s.\n", token_to_string[input->token], input->text);
        input = input->next;
    }
    fprintf(stderr, "Token: %s. Text: %s.\n", token_to_string[input->token], input->text);
    fprintf(stderr, "Together:\n");
    old_input = input;
    for(int i = 0; i < 11 && input != 0 && input->prev != 0; i++){
        input = input->prev;
    }
    while(input != old_input){
        fprintf(stderr, "%s ", input->text);
        if(input->token == TOKEN_SEMICOLON){
            fprintf(stderr, "\n");
        }
        input = input->next;
    }
    fprintf(stderr, "\x1b[31m%s\x1b[0m\n", input->text);
    exit(1);
}




bool is_literal(lex_token_t l){
    return l == TOKEN_LIT_INT || l == TOKEN_LIT_CHAR || l == TOKEN_LIT_STRING || l == TOKEN_LIT_TRUE || l == TOKEN_LIT_FALSE;
}



struct ast_stmt *parse_statement(){
    /*
     * Statements currently can be:
     *  - if-else statements
     *  - for statements
     *  - code blocks
     *  - local declarations
     *  - expressions (of any kind)
     *  - return statements
     *
     * We require a potential lookahead token to determine if it's a declaration.
     * Dangling else is a non-issue here: else is assigned to the last if.
     */

    struct ast_stmt *res = 0;
    lex_token_t cur_token = peek_token();
    while(cur_token == TOKEN_SEMICOLON){ //allow the empty statement
        advance_token();
        cur_token = peek_token();
    }
    if(cur_token == TOKEN_KW_PRINT){
        //For now, only allow one print expression
        advance_token();
        res = create_stmt_print(parse_expression(0));
        consume_token(TOKEN_SEMICOLON);
    }else if(cur_token == TOKEN_KW_IF){
        advance_token();
        consume_token(TOKEN_LPAREN);
        struct ast_expr *cond = parse_expression(0);
        consume_token(TOKEN_RPAREN);
        struct ast_stmt *main_code = parse_statement();
        if(peek_token() == TOKEN_KW_ELSE) {
            advance_token();
            struct ast_stmt *else_code = parse_statement();
            res = create_stmt_ifelse(cond, main_code, else_code);
        }else{
            res = create_stmt_ifelse(cond, main_code, 0);
        }
    }else if(cur_token == TOKEN_KW_FOR){
        advance_token(); //consume FOR
        consume_token(TOKEN_LPAREN);

        struct ast_expr *init = 0;
        if(peek_token() != TOKEN_SEMICOLON){
            init = parse_expression(0);//Note: C allows declarations here ("clause-1"), but B minor does not.
        }
        consume_token(TOKEN_SEMICOLON);
        struct ast_expr *main_cond = 0;
        if(peek_token() != TOKEN_SEMICOLON){
            main_cond = parse_expression(0);
        }
        consume_token(TOKEN_SEMICOLON);
        struct ast_expr *next_expr = 0;
        if(peek_token() != TOKEN_RPAREN){
            next_expr = parse_expression(0);
        }
        consume_token(TOKEN_RPAREN);
        auto main_code = parse_statement();
        res = create_stmt_for(init, main_cond, next_expr, main_code);

    }else if(cur_token == TOKEN_LBRACE){
        //begin code block
        //this is the same code as for function declaration
        advance_token(); //eat LBRACE
        struct ast_stmt* code = 0;
        struct ast_stmt* goer = 0;
        while(peek_token() != TOKEN_RBRACE){
            if(goer) {
                goer->next_stmt = parse_statement();
                goer = goer->next_stmt;
            }else{
                code = parse_statement();
                goer = code;
            }
        }
        if(!code){
            code = create_stmt_empty_block();
        }
        res = create_stmt_blck(code);
        advance_token(); //eat RBRACE
    }else if(is_literal(cur_token)){
        printf("I can't think of any useful situation where a statement should begin with a literal, actually.\n");
        auto main_expr = parse_expression(0);
        consume_token(TOKEN_SEMICOLON);
        res = create_stmt_expr(main_expr);
    }else if(cur_token == TOKEN_ID){
        //Either a declaration or an expression (this time they're useful because of function calls)
        if(peek_next_token() == TOKEN_COLON){
            //It's a declaration
            auto dec = parse_decl();
            res = create_stmt_decl(dec);
        }else{
            auto main_expr = parse_expression(0);
            consume_token(TOKEN_SEMICOLON);
            res = create_stmt_expr(main_expr);
        }
    }else if(cur_token == TOKEN_KW_RETURN){
        advance_token();
        //Allow for empty return.
        if(peek_token() != TOKEN_SEMICOLON){
            auto main_expr = parse_expression(0);
            consume_token(TOKEN_SEMICOLON);
            res = create_stmt_return(main_expr);
        }else{
            advance_token();
            res = create_stmt_return(0);
        }
    }else if(cur_token == TOKEN_EOF){
        parser_fail_print("Hit the end of file while parsing statement(s).\n");
        exit(1);
    }else{
        parser_fail_print("No support for token %s currently in statements.\n", token_to_string[cur_token]);
        exit(1);
    }
    return res;
}


#define I_SUCK_AT_NESTED_ARRAYS false

struct ast_vtype *parse_type(bool is_in_array, bool is_in_function, bool is_in_params){
    //array is right-recursive, and function loops, so nothing hard here
    //could make this a jump table but...
    //but just to talk about it:
    //I would create __SPECIAL_BEGIN_KW_TYPES and __SPECIAL_END_KW_TYPES in token.h between the types in the enum
    //and then check what it's between
    struct ast_vtype *res;
    lex_token_t l = input->token;
    if(l == TOKEN_KW_INT){
        res = create_ast_vtype_normal(VTYPE_INT);
        advance_token();
    }else if(l == TOKEN_KW_BOOL){
        res = create_ast_vtype_normal(VTYPE_BOOL);
        advance_token();
    }else if(l == TOKEN_KW_CHAR){
        res = create_ast_vtype_normal(VTYPE_CHAR);
        advance_token();
    }else if(l == TOKEN_KW_STRING){
        res = create_ast_vtype_normal(VTYPE_STRING);
        advance_token();
    }else if(l == TOKEN_KW_VOID){
        if(!is_in_function){
            parser_fail_print("(Semantic) fail: voids are only for function return.\n");
        }
        res = create_ast_vtype_normal(VTYPE_VOID);
        advance_token();
    }else if(l == TOKEN_KW_ARRAY){
        if(is_in_function){
            parser_fail_print("(Semantic) fail: we don't (yet) support putting arrays in function return.\n");
        }
#if I_SUCK_AT_NESTED_ARRAYS
        if(is_in_array){
            //Because I haven't syntactically defined initializing arrays in arrays.
            parser_fail_print("Sorry, but I don't know how to support arrays in arrays yet...\n");
        }
#endif
        advance_token(); //consume ARRAY
        consume_token(TOKEN_LBRACK);
        struct Token * arr_sz_token = consume_token(TOKEN_LIT_INT);
        int arr_sz = atoi(arr_sz_token->text);
        if(arr_sz < 0){
            parser_fail_print("(Semantic) fail: array size < 0.\n");
        }
        consume_token(TOKEN_RBRACK);
        res = create_ast_vtype_array(arr_sz, parse_type(true, is_in_function, is_in_params));
    }else if(l == TOKEN_KW_FUNCTION){
        advance_token(); //consume FUNCTION
        if(is_in_array || is_in_function || is_in_params){
            parser_fail_print("(Semantic) fail: we don't (yet) support putting functions inside of arrays or param list, or returning them.\n");
        }
        //Get the return type..
        struct ast_vtype *ret_vtype = parse_type(is_in_array, true, is_in_params);
        bool first_run = true;
        consume_token(TOKEN_LPAREN);
        struct ast_param_list *args = 0;
        struct ast_param_list *goer = 0;
        while(peek_token() != TOKEN_RPAREN){
            while(first_run || peek_token() == TOKEN_COMMA){
                if(!first_run) advance_token(); //consume COMMA
                struct Token *arg_id = consume_token(TOKEN_ID);
                consume_token(TOKEN_COLON);
                struct ast_vtype *arg_type = parse_type(is_in_array,is_in_function,true);
                if(peek_token() != TOKEN_COMMA && peek_token() != TOKEN_RPAREN){
                    parser_fail_print("In function params, failed to continue writing args.\n");
                }
                struct ast_param_list *next_arg = create_param_arg(arg_id->text, arg_type);
                if(goer){
                    goer->next = next_arg;
                    goer = goer->next;
                }else{
                    args = next_arg;
                    goer = args;
                }
                first_run = false;
            }
        }
        advance_token(); //consume RPAREN
        res = create_ast_vtype_fn(ret_vtype, args);

    }else{
        parser_fail_print("Expected a type but got %s\n", token_to_string[l]);
    }
    return res;
}

struct ast_decl *parse_decl(){
    while(peek_token() == TOKEN_SEMICOLON) advance_token(); //empty declarations are permitted
    struct ast_decl *res;
    struct Token *id = consume_token(TOKEN_ID);
    consume_token(TOKEN_COLON);
    struct ast_vtype *type = parse_type(false, false, false);
    if(type->kind == VTYPE_STRING && peek_token() != TOKEN_EQUALS){
        parser_fail_print("(sem fail): all strings must be initialized.\n");
    }
    if(peek_token() == TOKEN_EQUALS){
        //Initialization in general
        advance_token();
        if(type->kind == VTYPE_FN){
            //Function initialization
            consume_token(TOKEN_LBRACE);
            struct ast_stmt* code = 0;
            struct ast_stmt* goer = 0;
            while(peek_token() != TOKEN_RBRACE){
                if(goer) {
                    goer->next_stmt = parse_statement();
                    goer = goer->next_stmt;
                }else{
                    code = parse_statement();
                    goer = code;
                }
            }
            if(!code){
                if(type->subtype->kind != VTYPE_VOID){
                    parser_fail_print("(Semantic fail): Empty code for non-void type.\n");
                }
                code = create_stmt_empty_block(); //For void "pass" functions
            }
            consume_token(TOKEN_RBRACE);
            res = create_ast_decl(id->text, type, 0, code, 0);
        }else if(type->kind == VTYPE_ARRAY){
            //Array initialization
            consume_token(TOKEN_LBRACE);
            bool first_run = true;
            struct ast_expr *arr_list = 0;
            struct ast_expr *goer = 0;
            while(peek_token() != TOKEN_RBRACE){
                while(first_run || peek_token() == TOKEN_COMMA){
                    if(!first_run) advance_token(); //consume COMMA
                    auto a_member = create_expr_arr_member(parse_expression(0));
                    if(peek_token() != TOKEN_COMMA && peek_token() != TOKEN_RBRACE){
                        parser_fail_print("In array init, failed to continue listing members.\n");
                    }
                    if(goer){
                        goer->right = a_member;
                        goer = goer->right;
                    }else{
                        arr_list = a_member;
                        goer = a_member;
                    }
                    first_run = false;
                }
            }
            advance_token(); //consume RBRACE
            res = create_ast_decl(id->text, type, arr_list, 0, 0);

        }else{
            struct ast_expr* expr = parse_expression(0);
            res = create_ast_decl(id->text, type,  expr, 0, 0);
            consume_token(TOKEN_SEMICOLON);
        }
    }else if(type->kind == VTYPE_FN && peek_token() == TOKEN_LBRACE){
        //Allow function initialization without equals
        consume_token(TOKEN_LBRACE);
        struct ast_stmt* code = 0;
        struct ast_stmt* goer = 0;
        while(peek_token() != TOKEN_RBRACE){
            if(goer) {
                goer->next_stmt = parse_statement();
                goer = goer->next_stmt;
            }else{
                code = parse_statement();
                goer = code;
            }
        }

        if(!code){
            if(type->subtype->kind != VTYPE_VOID){
                parser_fail_print("(Semantic fail): Empty code for non-void type.\n");
            }
            code = create_stmt_empty_block(); //For void "pass" functions
        }
        consume_token(TOKEN_RBRACE);
        res = create_ast_decl(id->text, type, 0, code, 0);
    }else{
        res = create_ast_decl(id->text, type, 0, 0, 0);
        consume_token(TOKEN_SEMICOLON);
    }
    return res;
}

struct ast_decl *parse_program(FILE *fp){
    stream = fp;
    input = 0; //null
    advance_token(); //like feeding the tape
    struct ast_decl *ans = 0;
    struct ast_decl *goer = 0;
    while(input->token != TOKEN_EOF){
        if(ans){
            goer->next = parse_decl();
            goer = goer->next;
        }else{
            ans = parse_decl();
            goer = ans;
        }
    }
    return ans;
}
