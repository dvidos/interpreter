#ifndef _STR_OBJECT_H
#define _STR_OBJECT_H

#include "../framework/objects.h"

extern type_object *str_object;

object *new_str_object(const char *value);
const char *str_object_as_char_ptr(object *obj);


#endif
