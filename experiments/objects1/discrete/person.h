#ifndef _PERSON_H
#define _PERSON_H

#include "../framework/struct_info.h"


// reusable base attributes for child classes
#define PERSON_INSTANCE_ATTRIBUTES(vtable_type)  \
    BASE_INSTANCE_ATTRIBUTES(vtable_type); \
    /* remaining instance properties */  \
    const char *name

// reusable base methods for child classes
#define PERSON_VTABLE_METHODS(instance_type)  \
    void (*dream)(instance_type *p)



// two structures: instance and methods vtable
typedef struct person person;
typedef struct person_vtable person_vtable;

struct person {
    PERSON_INSTANCE_ATTRIBUTES(person_vtable);
};

struct person_vtable {
    PERSON_VTABLE_METHODS(person);
};

// public instance of class info
extern struct_info *person_struct;

// constructor(s)
person *new_person(const char *name);


#endif
