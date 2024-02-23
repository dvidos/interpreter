#ifndef _INT_OBJECT_H
#define _INT_OBJECT_H

#include "../framework/objects.h"

extern object_type *int_object;

object *new_int_object(int value);
int int_object_as_int(object *obj);


#endif
