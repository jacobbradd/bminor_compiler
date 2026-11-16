#ifndef __STACKI_H
#define __STACKI_H

typedef struct stacki{
    int * arr;
    int sz;
    int max_sz;
}stacki;

stacki * stacki_create();
void stacki_push(stacki * s, int i);
int stacki_pop(stacki *s);
int stacki_top(stacki *s);
void stacki_free(stacki *s);

#endif
