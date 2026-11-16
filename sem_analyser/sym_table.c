
#include "sym_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ast.h"

//Not declared in header

void _sym_free(struct Symbol *s){
    free(s);
}

void _sym_list_free(struct SymbolList *sl){
    for(int i = 0; i < sl->sz; i++){
        if(sl->sym_list[i]) _sym_free(sl->sym_list[i]);
    }
    free(sl);
}

uint32_t _hash_fn(char *name, unsigned size){
    //This is FNV plus some bad murmur-style xoring
    //size is assumed to be a power of 2
    uint64_t res = 0xcbf29ce484222325;
    for(int i = 0; name && name[i]; i++){
        res ^= name[i];
        res *= 0x00000100000001b3;
    }
    uint32_t inp = (uint32_t)(res ^ (res >> 32));
    inp ^= inp >> 16;
    inp *= 0x85ebca6b;
    inp ^= inp >> 13;
    inp *= 0xc2b2ae35;
    inp ^= inp >> 16;
    return inp & (size - 1);

}

void _sym_list_resize(struct SymbolList *sl){
    int old_max = sl->max_sz;
    sl->max_sz *= 2;
    struct Symbol* *new_world = calloc(sl->max_sz, sizeof(*(sl->sym_list)));
    for(int i = 0; i < old_max; i++){
        if(sl->sym_list[i]){
            uint32_t lookup = _hash_fn(sl->sym_list[i]->name, sl->max_sz);
            while(new_world[lookup]) {
                lookup = (lookup + 1)%(sl->max_sz);
            }
            new_world[lookup] = sl->sym_list[i];
        }
    }
    free(sl->sym_list);
    sl->sym_list = new_world;
}

//Declared in header


struct SymbolTable *create_sym_table(){
    struct SymbolTable *res = calloc(1, sizeof(*res));
    res->sym_stack = malloc(1 * sizeof(*(res->sym_stack)));
    res->sz = 0;
    res->max_sz = 1;
    res->reg_count = 0;
    return res;
}


struct SymbolList *sym_table_get(struct SymbolTable *st, int index){
    return st->sym_stack[index];
}

void sym_table_enter(struct SymbolTable *st){
    auto sl = create_sym_list();
    sym_table_push(st, sl);
}

void sym_table_push(struct SymbolTable *st, struct SymbolList* sl){
    if(st->sz >= st->max_sz/2){
        st->sym_stack = realloc(st->sym_stack, st->max_sz*2*sizeof(*(st->sym_stack)));
        st->max_sz *= 2;
    }
    st->sym_stack[st->sz] = sl;
    st->sz++;
}

void sym_table_pop(struct SymbolTable *st){
    if(st->sz > 0){
        _sym_list_free(st->sym_stack[st->sz-1]);
        st->sz--;
        if(st->sz == 1){
            st->reg_count = 0;
        }
    }else{
		fprintf(stderr, "Runtime error: sym_table_pop popped empty.\n");
        exit(1);
    }
}


struct SymbolList *create_sym_list(){
    struct SymbolList *res = calloc(1, sizeof(*res));
    res->sym_list = calloc(16, sizeof(*(res->sym_list)));
    res->sz = 0;
    res->max_sz = 16;
    return res;
}


void sym_list_insert(
    struct SymbolTable *st,
    char *name,
    int scope_level,
    struct ast_vtype *type,
    bool is_param,
    bool is_defined
){
    auto sl = sym_table_get(st, scope_level);
    if(sl->sz >= 3*sl->max_sz/4-1){
        _sym_list_resize(sl);
    }
    uint32_t lookup = _hash_fn(name, sl->max_sz);
    while(sl->sym_list[lookup] && strcmp(sl->sym_list[lookup]->name, name)){
        lookup = (lookup + 1) % sl->max_sz;
    }
    struct Symbol *s = calloc(1, sizeof(*s));
    if(sl->sym_list[lookup] == 0){
        s->reg_pos = st->reg_count;
        if(st->sz > 1) {
            if(is_param){
                st->num_args++;
            }else{
                st->reg_count++;
            }
        }
    }
    s->name = name;
    s->scope_level = scope_level;
    s->type = type;
    s->is_param = is_param;
    if(s->is_param) {
        s->param_pos = st->num_args;
    }
    s->is_defined = is_defined;
    sl->sym_list[lookup] = s;
    sl->sz++;
}

bool sym_list_find(struct SymbolList *st, char *name){
    uint32_t lookup = _hash_fn(name, st->max_sz);
    while(st->sym_list[lookup] && strcmp(st->sym_list[lookup]->name, name)){
        lookup = (lookup + 1) % st->max_sz;
    }
    if(!st->sym_list[lookup]){
        return false;
    }
    return true;
}

struct Symbol *sym_list_get(struct SymbolList *st, char *name){
    uint32_t lookup = _hash_fn(name, st->max_sz);
    while(st->sym_list[lookup] && strcmp(st->sym_list[lookup]->name, name)){
        lookup = (lookup + 1) % st->max_sz;
    }
    if(!st->sym_list[lookup]){
        printf("Runtime error: sym_list_get failed to find anything.\n");
        exit(1);
    }
    return st->sym_list[lookup];
}


struct Symbol * sym_table_lookup(struct SymbolTable *st, char* name){
    int stack_lvl = st->sz - 1;
    while(stack_lvl >= 0 && !sym_list_find(sym_table_get(st, stack_lvl), name)){
        stack_lvl--;
    }
    if(stack_lvl >= 0){
        return sym_list_get(sym_table_get(st, stack_lvl), name);
    }else{
        return 0;
    }
}
