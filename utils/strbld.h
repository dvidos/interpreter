#ifndef _STRBLD_H
#define _STRBLD_H


typedef struct strbld strbld;

strbld *new_strbld();

void strbld_cat(strbld *sb, const char *str);
void strbld_catc(strbld *sb, char chr);
void strbld_catf(strbld *sb, char *fmt, ...);

const char *strbld_charptr(strbld *sb);

#endif
