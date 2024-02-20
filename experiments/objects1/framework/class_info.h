#ifndef _CLASS_INFO_H
#define _CLASS_INFO_H

#include <stdbool.h>
#include <stdlib.h>
#include "str_builder.h"

// all instances have a pointer to an instance below
typedef struct class_info class_info;

// useful macro for when defining any instance struct
// at least the class info MUST be the first in the instance
#define BASE_INSTANCE_ATTRIBUTES(vtable_type)  \
    class_info *_class;  \
    vtable_type *_vt

typedef void (*describe_func)(void *instance, str_builder *sb);
typedef bool (*equals_func)(void *instance_a, void *instance_b);
typedef unsigned (*hash_func)(void *instance);
typedef void *(*clone_func)(void *instance);
typedef int (*compare_func)(void *instance_a, void *instance_b);
typedef void (*destruct_func)(void *instance);

struct class_info {
    const char *name;
    class_info *parent;
    
    describe_func describe;
    equals_func equals;
    hash_func hash;
    clone_func clone;
    compare_func compare;
    destruct_func destruct;
    
    unsigned int _class_info_magic_number;
};

#define CLASS_INFO_MAGIC_NUMBER   0xC1A55DEF

class_info *get_instance_class(void *instance);
const char *class_name(void *instance);
bool is_of_class_or_subclass(void *instance, class_info *c);
void class_describe(void *instance, str_builder *sb);
bool class_equals(void *instance_a, void *instance_b);
unsigned class_hash(void *instance);
void *class_clone(void *instance);
int class_compare(void *instance_a, void *instance_b);
void class_destruct(void *instance);

unsigned class_generic_hash(unsigned hash_value, const void *data_ptr, int data_bytes);

#endif
