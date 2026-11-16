
#include "stacki.h"
#include <stdlib.h>
#include <stdio.h>

stacki * stacki_create(){
    stacki * res = calloc(1, sizeof(*res));
    res->arr = malloc(4 * sizeof(int));
    res->max_sz = 4;
    return res;
}

void stacki_push(stacki * res, int i){
    if(res->sz >= res->max_sz/2){
        res->arr = realloc(res->arr, 2 * res->max_sz * sizeof(int));
        res->max_sz *= 2;
    }
    res->arr[res->sz] = i;
    res->sz++;
}
int stacki_pop(stacki *res){
    if(res->sz == 0){
        fprintf(stderr, "Popped an empty stacki.\n");
        exit(1);
    }
    res->sz--;
    return res->arr[res->sz];
}

int stacki_top(stacki *res){
    return res->arr[res->sz-1];
}

void stacki_free(stacki *s){
    free(s->arr);
    free(s);
}
