#ifndef __PARSER_H
#define __PARSER_H

#include "ast.h"
#include "lexer/tokens.h"
#include <stdio.h>
#include <stdlib.h>

struct Token{
    lex_token_t token;
    char * text;
    struct Token *next;
    struct Token *prev;
};


struct ast_expr_params{
    bool is_arr_decl;
};

struct ast_decl *parse_decl();
struct ast_decl *parse_program(FILE * fp);
struct ast_stmt *parse_statement();
struct ast_expr *parse_expression(int);
struct ast_vtype *parse_type(bool is_in_array, bool is_in_function, bool is_in_params);






#endif
