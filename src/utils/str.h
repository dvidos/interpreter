#ifndef _STR_H
#define _STR_H


typedef struct str str;

str *new_str();

void str_clear(str *s);
void str_add(str *s, const char *cstr);
void str_addc(str *s, char chr);
void str_addf(str *s, char *fmt, ...);

const char *str_cstr(str *s);

void str_free(str *s);

#endif
