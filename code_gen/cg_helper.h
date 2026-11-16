
#ifndef __CG_HELPER_H
#define __CG_HELPER_H

#include "ast.h"
#include "dag_simple.h"
#include "sym_table.h"
#include "data_types/string_t.h"
#include "data_types/map.h"


void code_op_dc(string * s, char* op, struct dag_node * dag1, char *dest, int fn_decls);
void code_op_cd(string * s, char* op, char* val, struct dag_node * dag2, int fn_decls);
void code_op_dd(string * s, char* op, struct dag_node * dag1, struct dag_node * dag2, int fn_decls);
void code_op_sc(string * s, char* op, int scratch1, char *dest, int fn_decls);
void code_op_cs(string * s, char* op, char* val, int scratch2, int fn_decls);
void code_op_ss(string * s, char* op, int scratch1, int scratch2, int fn_decls);
void code_op_cc(string * s, char * op, char* val, char *dest);
void code_label_c(string *s, char* label);
void code_label_i(string *s, int label);
void code_jump_label_c(string *res, char* op, char* label);
    void code_jump_label_i(string *res, char* op, int label);
void code_op_dn(string *s, char *op, struct dag_node *dag1, char* name2, struct SymbolTable *st, int fn_decls);
void code_op_cn(string *s, char *op, char *val, char* name2, struct SymbolTable *st);
void code_op_nc(string *s, char *op, char *name1, char* dest, struct SymbolTable *st);


char *scratch_register(int pos, int fn_decls);

char *symbol_code_gen(struct Symbol *s);
char *symbol_code_gen_with_offset(struct Symbol *s, struct dag_node * dag, int fn_decls);



int new_label();
char * get_label(int label_num);
char * get_local_var(int pos);
int add_ro(string * ro_buffer, hash_int * ro_string_holder, char * s);
void string_add_label(string *s, int label_num);
void string_add_local_var(string *s, int label_num);

#endif
