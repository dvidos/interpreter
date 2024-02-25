#ifndef _STUDENT_H
#define _STUDENT_H

#include "../framework/struct_info.h"
#include "person.h"


// two structures: instance and methods vtable
typedef struct student student;
typedef struct student_vtable student_vtable;

// instance structure
struct student {
    // cannot embed base class, as the vtable is strongly typed.
    PERSON_INSTANCE_ATTRIBUTES(student_vtable); // inherit from base type
    int grade;
};

// methods structure
struct student_vtable {
    PERSON_VTABLE_METHODS(student); // inherit from base type
    void (*take_test)(student *s, int grade);
};

// public instance of class info
extern struct_info *student_class;

// public constructor
student *new_student(const char *name);


#endif
