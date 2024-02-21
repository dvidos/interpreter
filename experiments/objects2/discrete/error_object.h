#ifndef _ERR_OBJECT_C
#define _ERR_OBJECT_C

#include "../framework/objects.h"

extern type_object *err_type;

object new_error_object_with_message(const char *msg, ...);
const char *error_object_as_message(object *);


#endif
