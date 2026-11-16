
#include "string_t.h"
#include <stdio.h>
#include <stdlib.h>

string *string_create_empty(){
    string *res = calloc(1, sizeof(*res));
    res->charr = calloc(4, sizeof(char));
    res->max_sz = 4;
    return res;
}
void string_push_back(string *s, char c){
    if(s->sz >= s->max_sz / 2){
        s->charr = realloc(s->charr, s->max_sz * 2 * sizeof(char));
        s->max_sz *= 2;
    }
    s->charr[s->sz] = c;
    s->charr[s->sz+1] = 0; //just in case :)
    s->sz++;
}

void string_append_c(string*s, char * c){
    int i = 0;
    while(c[i] != 0){
        string_push_back(s, c[i]);
        i++;
    }
}
void string_append(string *s, string * t){
    for(int i = 0; i < t->sz; i++){
        string_push_back(s,t->charr[i]);
    }
}
void string_print(string *s){
    printf("%s", s->charr);
}

void string_free(string *s){
    free(s->charr);
    free(s);
}
