#ifndef _STR_OBJECT_H
#define _STR_OBJECT_H

#include "../framework/objects.h"

extern object_type *str_object;

object *new_str_object(const char *fmt, ...);
const char *str_object_as_char_ptr(object *obj);


#endif
