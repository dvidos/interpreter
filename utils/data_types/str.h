#ifndef _STR_H
#define _STR_H

#include "../containers/_module.h"


typedef const char str;

extern contained_item *containing_strs;

bool strs_are_equal(str *a, str *b);
const char *str_to_string(str *s);


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(str);


#endif
