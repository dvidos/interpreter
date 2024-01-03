#ifndef _STRBUFF_H
#define _STRBUFF_H


typedef struct strbuff strbuff;

strbuff *new_strbuff();

void strbuff_cat(strbuff *s, const char *str);
void strbuff_catc(strbuff *s, char chr);
void strbuff_catf(strbuff *s, char *fmt, ...);

const char *strbuff_charptr(strbuff *s);

#endif
