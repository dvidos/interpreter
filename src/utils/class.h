#ifndef _CLASS_H
#define _CLASS_H

#include <stdbool.h>
#include "str_builder.h"


typedef bool (*are_equal_func)(void *pointer_a, void *pointer_b);
typedef void (*describe_func)(void *pointer, str_builder *sb);
typedef unsigned long *(*hash_func)(void *pointer);

typedef struct class {
    const char *type_name;
    are_equal_func are_equal;
    describe_func  describe;
    hash_func      hash;
    // also: clone, compare, serialize, unserialize, etc.
} class;

bool classes_are_equal(class *a, class *b);


#endif
