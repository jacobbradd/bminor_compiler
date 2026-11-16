#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include "b_minor_lexer.h"
extern FILE *yyin;
extern int yylex();
extern char *yytext;



unsigned compute_length(char * inp){
    unsigned res = 0;
    while(*inp){
        res++;
        inp++;
    }
    return res;
}

void make_lex_file() {
    yyin = fopen("sample_source.bm","r");
    FILE * output = fopen("/tmp/b_minor_lexed.l", "w");
    if(!yyin) {
        fprintf(stderr, "Could not open sample_source.bm!\n");
        exit(1);
    }
    while(1) {
        lex_token_t t = yylex();
        if(t==TOKEN_EOF) break;
        fprintf(output,"%03d %07d %s\n",t,compute_length(yytext),yytext);
    }
    fprintf(output,"%03d %07d \n", 0, 0);
    fprintf(output,"EOF\n");
    fclose(yyin);
    fclose(output);
}
