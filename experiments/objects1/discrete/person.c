#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "person.h"


// all private methods here
static void person_dream(person *p) {
    printf("I had a dream today...\n");
}

// all class methods here
static void person_describe(person *p, str_builder *sb) {
    str_builder_addf(sb, "Hi, I'm %s", p->name);
}

static unsigned person_hash(person *p) {
    return class_generic_hash(0, p->name, strlen(p->name));
}

// public instance of class info
class_info *person_class = &(class_info){
    .name = "person",
    .parent = NULL,
    ._class_info_magic_number = CLASS_INFO_MAGIC_NUMBER,
    .describe = (describe_func)person_describe,
    .hash = (hash_func)person_hash,
};

// private instance of the vtable
static person_vtable *person_vt = &(person_vtable){
    .dream = person_dream
};

// public constructor(s)
person *new_person(const char *name) {
    person *p = malloc(sizeof(person));
    p->_class = person_class;
    p->_vt = person_vt;
    p->name = name;
    return p;
}
