#ifndef _STR_H
#define _STR_H

#include "containers/_module.h"


typedef const char str;

extern contained_item *containing_strs;

bool strs_are_equal(str *a, str *b);
const void str_describe(str *s, str_builder *sb);


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(str);
#define failed_str(inner, fmt, ...)  __failed_str(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


#endif
