#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "student.h"


// all private methods here
static void student_dream(student *s) {
    printf("I took a test today...\n");
}
static void student_take_test(student *s, int grade) {
    s->grade = grade;
}

// all class methods here
static void student_describe(student *s, str_builder *sb) {
    str_builder_addf(sb, "Hi, my name is %s, my grade is %d", s->name, s->grade);
}

static unsigned student_hash(student *s) {
    unsigned u = 0;
    u = instance_generic_hash(u, s->name, strlen(s->name));
    u = instance_generic_hash(u, &s->grade, sizeof(s->grade));
    return u;
}

// public instance of class info
struct_info *student_class = &(struct_info){
    .struct_name = "student",
    // .enclosed = person_class, (we cannot initialize to another compile-time pointer)
    ._class_info_magic_number = CLASS_INFO_MAGIC_NUMBER,
    .describe = (describe_func)student_describe,
    .hash = (hash_instance_func)student_hash
};

// private instance of the vtable
static student_vtable *student_vt = &(student_vtable){
    .dream = student_dream,
    .take_test = student_take_test
};

// public constructor(s)
student *new_student(const char *name) {
    if (student_class->enclosed == NULL)
        student_class->enclosed = person_class;
    
    student *s = malloc(sizeof(student));
    s->_class = student_class;
    s->_vt = student_vt;
    s->name = name;
    s->grade = 50;
    return s;
}
