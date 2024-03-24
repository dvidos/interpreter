#ifndef _STR_H
#define _STR_H

#include <stdbool.h>


typedef struct contained_item_info contained_item_info;

typedef struct str str;
extern contained_item_info *str_item_info;


str *new_str();
str *new_str_of(const char *value);

bool str_empty(str *s);
int str_length(str *s);
void str_clear(str *s);
void str_add(str *s, str *other);
void str_adds(str *s, const char *cstr);
void str_addc(str *s, char chr);
void str_addf(str *s, char *fmt, ...);

bool str_equals(str *s, const char *value);
bool str_starts_with(str *s, const char *part);
bool str_ends_with(str *s, const char *part);
bool str_contains(str *s, const char *part);

str *str_substr(str *s, int start, int length);
str *str_trim(str *s, const char *delimiters);
str *str_get_token(str *s, int *start);

const char *str_cstr(str *s);
void str_free(str *s);


#endif
