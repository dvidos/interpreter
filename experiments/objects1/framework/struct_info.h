#ifndef _STRUCT_INFO_H
#define _STRUCT_INFO_H

#include <stdbool.h>
#include <stdlib.h>
#include "str_builder.h"

// all instances have a pointer to an instance below
typedef struct struct_info struct_info;

// useful macro for when defining any instance struct
// at least the class info MUST be the first in the instance
#define BASE_INSTANCE_ATTRIBUTES(vtable_type)  \
    struct_info *_info;  \
    vtable_type *_vt

typedef void (*describe_item_func)(void *instance, str_builder *sb);
typedef bool (*instances_are_equal_func)(void *instance_a, void *instance_b);
typedef unsigned (*hash_instance_func)(void *instance);
typedef void *(*clone_instance_func)(void *instance);
typedef int (*compare_instances_func)(void *instance_a, void *instance_b);
typedef void (*destruct_instance_func)(void *instance);

struct struct_info {
    const char *struct_name;
    struct_info *enclosed; // a.k.a. base class
    
    describe_item_func describe;
    instances_are_equal_func equals;
    hash_instance_func hash;
    clone_instance_func clone;
    compare_instances_func compare;
    destruct_instance_func destruct;
    
    unsigned int _struct_info_magic_number;
};

#define STRUCT_INFO_MAGIC_NUMBER   0x49DC37E6

struct_info *get_instance_info(void *instance);
const char *instance_struct_name(void *instance);
bool is_or_encloses_struct(void *instance, struct_info *c);
void instance_describe(void *instance, str_builder *sb);
bool instances_are_equal(void *instance_a, void *instance_b);
unsigned instance_hash(void *instance);
void *instance_clone(void *instance);
int instances_compare(void *instance_a, void *instance_b);
void instance_destruct(void *instance);

unsigned instance_generic_hash(unsigned hash_value, const void *data_ptr, int data_bytes);

#endif
