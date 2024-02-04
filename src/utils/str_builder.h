#ifndef _STR_BUILDER_H
#define _STR_BUILDER_H


typedef struct str_builder str_builder;

str_builder *new_str_builder();

void str_builder_clear(str_builder *sb);
void str_builder_add(str_builder *sb, const char *str);
void str_builder_addc(str_builder *sb, char chr);
void str_builder_addf(str_builder *sb, char *fmt, ...);

const char *str_builder_charptr(str_builder *sb);

#endif
