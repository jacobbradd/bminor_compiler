#ifndef __MAP_H
#define __MAP_H

#include "string_t.h"

typedef struct keyval_int{
    string* key;
    int val;
} keyval_int;

//hashtable is like a dynamic array but with hash accessing
typedef struct hash_int{
    int _size;
    int _max_size;
    keyval_int **_arr;
} hash_int;


hash_int* hash_int_create_empty();

void hash_int_insert(hash_int *ha, char * key, int val);
int hash_int_get_val(hash_int *ha, char * key);
void hash_int_free(hash_int *ha);
int hash_int_find(hash_int *ha, char * key);

#endif
