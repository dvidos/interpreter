#ifndef _STUDENT_H
#define _STUDENT_H

#include "../framework/struct_info.h"
#include "person.h"

#define STUDENT_ATTRIBUTES(vtable_type)  \
    PERSON_ATTRIBUTES(vtable_type); \
    int grade;

// reusable base methods for child classes
#define STUDENT_METHODS(instance_type)  \
    PERSON_METHODS(instance_type);    \
    void (*take_test)(student *s, int grade);



// two structures: instance and methods vtable
typedef struct student student;
typedef struct student_vtable student_vtable;

struct student {
    STUDENT_ATTRIBUTES(student_vtable); // inherit from base type
};

struct student_vtable {
    STUDENT_METHODS(student); // inherit from base type
};

// public instance of class info
extern struct_info *student_struct;

// public constructor
student *new_student(const char *name);


#endif
