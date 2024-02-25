#ifndef _SKELETON_H
#define _SKELETON_H

#include "../framework/struct_info.h"

/*
    At a glance:
    * define for instance attributes
    * define for vtable methods
    * typedef & declaration of the two structures
    * public class variable
    * public constructor
*/



// define instance attributes here, to allow others to encapsulate
#define SKELETON_INSTANCE_ATTRIBUTES(vtable_type)  \
    BASE_INSTANCE_ATTRIBUTES(vtable_type); \
    const char *skeleton_attr_1; \
    long skeleton_attr_2; \
    bool skeleton_attr_3;

// define vtable methods here, to allow others to encapsulate
#define SKELETON_VTABLE_METHODS(instance_type)  \
    void (*skeleton_method_1)(instance_type *p); \
    long (*skeleton_method_2)(instance_type *p, long number); \
    void (*skeleton_method_3)(instance_type *p, const char *ptr);

// typedef these two, then define them below
typedef struct skeleton skeleton;
typedef struct skeleton_vtable skeleton_vtable;

struct skeleton { // all instances of this type, contains data
    SKELETON_INSTANCE_ATTRIBUTES(skeleton_vtable);
};

struct skeleton_vtable { // one instance for this, contains methods
    SKELETON_VTABLE_METHODS(skeleton);
};

// the public "class" of the skeleton, for type checking
extern struct_info *skeleton_class;

// finally, a constructor, with possible arguments
skeleton *new_skeleton();


#endif
