#ifndef _CONTAINABLE_H
#define _CONTAINABLE_H

#include <stdbool.h>


// for something to be containable, the first member of the structure
// must point to a containable structure, whose magic number is correct.
// this allows for containers (list, dict, stack etc) 
// to check for equality or convert to string and print, 
// without knowing the type of the contained items

typedef struct containable containable;
typedef bool (*are_equal_func)(void *pointer_a, void *pointer_b);
typedef const char *(*to_string_func)(void *pointer);
typedef unsigned long *(*hash_func)(void *pointer);

containable *new_containable(const char *struct_name, are_equal_func are_equal, to_string_func to_string);

bool is_containable_instance(void *pointer);
bool containables_are_equal(void *pointer_a, void *pointer_b);
const char *containable_to_string(void *pointer);


typedef struct contained_item_info {
    are_equal_func are_equal;
    to_string_func to_string;
    hash_func      hash;
    // clone, compare, serialize, unserialize, etc.
} contained_item_info;

#endif
