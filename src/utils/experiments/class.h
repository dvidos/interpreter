#ifndef _CLASS_H
#define _CLASS_H

#include <stdbool.h>

typedef struct class class;

struct class {
    const char *name;
    
    const char *(*to_string)(void *instance);
    bool (*are_equal)(void *a, void *b);
    // int (*hash)(void *instance);
    // void *(*clone)(void *instance);
    // const char *serialize(void *instance);
    // bool unserialize(const char *data);
    
    unsigned int classdef_magic;
};

#define CLASSDEF_MAGIC   0xC1A55DEF


class *get_class(void *instance);
bool is_instance(void *instance, class *c);
const char *instance_to_string(void *instance);
bool instances_are_equal(void *a, void *b);


#endif

