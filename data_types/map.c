#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "map.h"

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}
uint32_t fnv_hash(char* a){
    //Here's "Fowler-Noll-Vo".
    //That said, my basic rolling hash + euler hash has lower variance?
    uint64_t res = 0xcbf29ce484222325;
    for(int i = 0; a[i]; i++){
        res *= 0x00000100000001b3; //note the integer overflow handles bitmasking
        res ^= a[i];
    }
    return (res >> 32) ^ (res & (((uint64_t)1 >> 32)-1)) ; //xor folding (not much diff?)
}

uint32_t murmur_hash(uint32_t inp, unsigned size){
    //this is like a small bit of murmur hash
    inp ^= inp >> 16;
    inp *= 0x85ebca6b;
    inp ^= inp >> 13;
    inp *= 0xc2b2ae35;
    inp ^= inp >> 16;
    return inp & (size - 1);

}

uint32_t roll_hash(char* a){

    //basic rolling hash
    uint64_t val_N= 1;
    uint64_t res = 1289361892; //learning: offsets are better probably
    uint64_t bitmask = (((uint64_t)1 << 32) - 1);
    for(int i = 0; a[i]; i++){
        res = (res + ((val_N * a[i]) & bitmask)) & bitmask;
        val_N = (val_N * 257) & bitmask;
    }
    return (uint32_t)res;
}


uint32_t mult_hash(uint32_t inp, unsigned size){


    //idea: half-decent random numbers are fractional part of k*a for k \in Z and a \in R
    //so then take floor(size * frac(ka))
    //to do quickly, consider frac(ka) in base 2, so it's 0.abcdefghijk
    //size * 0.abcdefghijk = size * abcdefghijk.0 >> something
    //so choose a good integer a, multiply by size, and divide by 2^roof(log_2 a)

    //fibonacci hash I assume means a = golden ratio
    //a * 2^32 = 6949403065.something
    //so... size * 6949403065 >> 32 ??

    //PRECONDITION: size is a power of 2
    //to be hipster, I'll try euler instead of golden ratio
    //uint64_t good_a = 6949403065;
    //uint64_t good_a = 2654435769; //actually they use 1/phi
    uint64_t good_a = 11674931554; //euler


    //High bits? Not sure if it's right yet
    //quickly compute log2 size
    int log = 0;
    int a = 1;
    while(a < size){
        a *= 2;
        log++;
    }

    uint64_t ninp = inp;
    ninp *= 12737178231;
    ninp ^= ninp >> (64 - log);
    return ((ninp * good_a) >> (64-log));
    //This computes the *low bits*
    uint64_t bitmask = size-1;
    uint64_t res = (((uint64_t)inp * good_a) >> (32)) & bitmask;
    return (uint32_t) res;
}

uint32_t _2hash_fn(char *name, unsigned size){
    //TODO use an actually good hash. This is FNV plus some bad murmur-style xoring
    //size is assumed to be a power of 2
    uint64_t res = 0xcbf29ce484222325;
    for(int i = 0; name && name[i]; i++){
        res *= 0x00000100000001b3;
        res ^= name[i];
    }
    uint32_t inp = (res >> 32) ^ (res & (((uint64_t)1 >> 32)-1)) ;
    inp ^= inp >> 16;
    inp *= 0x85ebca6b;
    inp ^= inp >> 13;
    inp *= 0xc2b2ae35;
    inp ^= inp >> 16;
    return inp & (size - 1);

}

uint32_t hash_fn(char* a, unsigned size){
    return _2hash_fn(a, size);
    //PRECONDITION: size is a power of 2 (for easy modding out tho)
    //return murmur_hash(fnv_hash(a), size);
    //return mult_hash(fnv_hash(a), size);
    return mult_hash(roll_hash(a), size);

}

bool my_strcmp(char *a, char *b){
    //checks if NOT same
    int i = 0;
    while(a[i] && b[i]){
        if(a[i] != b[i]) return true;
        i++;
    }
    return a[i] != b[i];
}


//==================================
//Actual Map code.

void __hash_int_double_rehash(hash_int *ha);

hash_int* hash_int_create_empty(){
    hash_int* res = malloc(1*sizeof(hash_int));
    res->_arr = calloc(16,sizeof(*(res->_arr)));
    res->_size = 0;
    res->_max_size = 16;
    return res;
}

void __hash_int_double_rehash(hash_int *ha){
    int old_max = ha->_max_size;
    ha->_max_size *= 2;
    keyval_int **new_world = calloc(ha->_max_size, sizeof(*new_world));
    for(int i = 0; i < old_max; i++){
        if(ha->_arr[i]){
            uint32_t res = hash_fn(ha->_arr[i]->key->charr, ha->_max_size);
            while(new_world[res]) res = (res + 1)%(ha->_max_size);
            new_world[res] = ha->_arr[i];
        }
    }
    free(ha->_arr);
    ha->_arr = new_world;
}

void hash_int_insert(hash_int *ha, char * key, int val){
    if(ha->_size >= 3*ha->_max_size/4 - 1){
        __hash_int_double_rehash(ha);
    }
    uint32_t res = hash_fn(key, ha->_max_size);
    while(ha->_arr[res] && strcmp(ha->_arr[res]->key->charr, key)) res = (res + 1)%ha->_max_size;
    keyval_int * kv = calloc(1, sizeof(*kv));
    kv->key= string_create_empty();
    string_append_c(kv->key, key);
    kv->val = val;
    ha->_arr[res] = kv;
    ha->_size++;
}

int hash_int_get_val(hash_int *ha, char * key){
    //PRECONDITION: key must exist
    //if it doesn't, well.. we could be here a while

    uint32_t res = hash_fn(key, ha->_max_size);
    //linear probe
    while(ha->_arr[res] && my_strcmp(ha->_arr[res]->key->charr, key)) res = (res + 1)%ha->_max_size;
    if(!ha->_arr[res]){
        printf("Runtime error: hash map failed to find anything. Code sux.\n");
        exit(1);
    }
    return ha->_arr[res]->val;
}
int hash_int_find(hash_int *ha, char * key){
    //PRECONDITION: key must exist
    //if it doesn't, well.. we could be here a while

    uint32_t res = hash_fn(key, ha->_max_size);
    //linear probe
    while(ha->_arr[res] && my_strcmp(ha->_arr[res]->key->charr, key)) res = (res + 1)%ha->_max_size;
    if(!ha->_arr[res]){
        return false;
    }
    return true;
}

void hash_int_free(hash_int *ha){
    for(int i = 0; i < ha->_size; i++){
        if(ha->_arr[i]){
            free(ha->_arr[i]);
        }
    }
    free(ha->_arr);
    free(ha);
    ha = 0;
}
