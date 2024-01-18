#ifndef _STR_BUILDER_H
#define _STR_BUILDER_H


typedef struct str_builder str_builder;

str_builder *new_str_builder();

void str_builder_cat(str_builder *sb, const char *str);
void str_builder_catc(str_builder *sb, char chr);
void str_builder_catf(str_builder *sb, char *fmt, ...);

const char *str_builder_charptr(str_builder *sb);

#endif
