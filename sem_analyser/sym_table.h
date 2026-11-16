
#ifndef __SYM_TABLE_H
#define __SYM_TABLE_H

#include <stdint.h>
#include "ast.h"

struct Symbol{
    char *name;
    int scope_level; //0 means global. This is redundant.
    struct ast_vtype *type;
    bool is_param;
    bool is_defined; //declarations
    int reg_pos;
    int param_pos;
};

//Hash
struct SymbolList{
    struct Symbol* *sym_list;
    unsigned sz;
    unsigned max_sz;
};

//Stack of SymbolList: really an array due to needing random access for name resolution
struct SymbolTable{
    struct SymbolList* *sym_stack;
    struct ast_vtype *ret_vtype; //for function scope
    unsigned sz;
    unsigned max_sz;
    int reg_count;
    int num_args;
};

struct SymbolTable *create_sym_table();
struct SymbolList *sym_table_get(struct SymbolTable *st, int index);
void sym_table_enter(struct SymbolTable *st);
void sym_table_push(struct SymbolTable *st, struct SymbolList* sl);
void sym_table_pop(struct SymbolTable *st);
struct SymbolList *create_sym_list();
void sym_list_insert(
    struct SymbolTable *st,
    char *name,
    int scope_level,
    struct ast_vtype *type,
    bool is_param,
    bool is_defined
);

bool sym_list_find(struct SymbolList *st, char *name);
struct Symbol *sym_list_get(struct SymbolList *st, char *name);


void sym_list_free(struct SymbolList *sl);
struct Symbol * sym_table_lookup(struct SymbolTable *st, char* name);

#endif
