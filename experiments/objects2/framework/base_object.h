#ifndef _BASE_OBJECT_H
#define _BASE_OBJECT_H


// all "object" structures (including the type_object) 
// must have these as the first items, to allow uniform treatment
#define FIRST_OBJECT_ATTRIBUTES        \
            struct type_object *type;  \
            int references_count


// for statically allocated objects, we don't need to count references
// so use this define for the `references_count` attribute
#define OBJECT_STATICALLY_ALLOCATED   (-2)


// all objects can be cast to this pointer
// essentially, they can all pretend to be this structure
// check the "type" attribute, to find which "subclass" they are
struct object { // all objects children of this
    FIRST_OBJECT_ATTRIBUTES;
};


#endif
