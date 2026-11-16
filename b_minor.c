/*
 *
 * b_minor.c
 *
 * So far, the lexer and parser have been constructed.
 * Update: the code generator has been completed (mostly)!
 *
 * TODO
 * 1. Implement print. This is best done with a generator specifically for calling printf. DONE
 * 2. CF graph printer and reader.
 * 3. Control flow analysis/data flow analysis optimizations.
 * 4. Peephole optimizations on generated code.
 * 5. Another intermediate representation: linear IR.
 * 6. Maybe pointers aren't necessary? Java doesn't use them
 * 7. Structs, classes...
 * 8. Concatenate string literals like C does.
 *
 *
 * TODO certain features are not yet supported in the lexer and parser. This is partly due to my laziness in managing large enums/arrays.
 * It would help if I wrote a script which took the list of tokens and specifying which are ops and so on, and output the relevant array code.
 *
 * - Separating function declaration and intitialization is a semantic issue (the syntax already supports this, even for variables).
 * - Support auto type
 * - Support pointers
 * - Support structs? Parsing is easy: struct id {list of "parameters" like function args} assigns the struct name to optional id, else anonymous
 * - missing operators == and !=
 *
 * TODO I check if a constant is 0 during division, but what about actual division by 0?
 *
 *
 */


#include <stdio.h>
#include <stdarg.h>

#include "lexer/b_minor_lexer.h"
#include "ast.h"
#include "parser.h"
#include "sem_analyser.h"
#include "dag_simple.h"
#include "code_gen.h"
#include "data_types/string_t.h"

int main(){
#ifdef __TEST_SUITE_H
    do_tests();
    return 0;
#endif
    make_lex_file();
    FILE * stream = fopen("/tmp/b_minor_lexed.l", "r");
    struct ast_decl *program_ast = parse_program(stream);
    printf("Parse success.\n");
    sem_analyse(program_ast);
    printf("Passed type-checking.\n");
    struct control_flow_node *cf = cf_gen(program_ast);
    printf("Created the control flow successfully: %p.\n", cf);
    auto compiled_code = code_gen(cf);
    printf("Succesfully compiled!\n");
    auto outf = fopen("compiled_code.s", "w");
    fprintf(outf, "%s", compiled_code->charr);
    return 0;
}
