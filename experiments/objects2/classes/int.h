#ifndef _INT_OBJECT_H
#define _INT_OBJECT_H

#include "../framework/objects.h"

extern type_object *int_type_object;

object *new_int_object_from_int(int value);
int int_object_as_int(object *obj);

