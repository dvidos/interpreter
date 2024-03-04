#ifndef _SKELETON_H
#define _SKELETON_H

#include "../framework/objects.h"

// all that's needed is this type
extern object_type *skeleton_type;


// specific constructors as needed. otherwise use object_create()
object *new_skeleton_from_value(int value);



#endif