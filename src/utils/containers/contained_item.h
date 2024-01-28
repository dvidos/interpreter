#ifndef _CONTAINABLE_H
#define _CONTAINABLE_H

#include <stdbool.h>


typedef bool (*are_equal_func)(void *pointer_a, void *pointer_b);
typedef const char *(*to_string_func)(void *pointer);
typedef unsigned long *(*hash_func)(void *pointer);

typedef struct contained_item {
    const char *type_name;
    are_equal_func are_equal;
    to_string_func to_string;
    hash_func      hash;
    // also: clone, compare, serialize, unserialize, etc.
} contained_item;

bool contained_item_info_are_equal(contained_item *a, contained_item *b);


#endif
