
#ifndef __STRING_H
#define __STRING_H

typedef struct string{
    char * charr;
    int sz;
    int max_sz;
} string;


string *string_create_empty();
void string_push_back(string *s, char c);

void string_append_c(string*s, char * c);

void string_append(string *s, string * t);

void string_print(string *s);
void string_free(string *s);


#endif
