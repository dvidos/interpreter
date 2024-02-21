#ifndef _ERR_OBJECT_C
#define _ERR_OBJECT_C

#include "../framework/objects.h"

extern type_object *error_object;

object *new_error_object(const char *msg, ...);
const char *error_object_as_charptr(object *obj);


#endif
