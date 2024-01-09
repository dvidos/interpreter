#include <stdlib.h>
#include <string.h>
#include "containable.h"

struct containable {
    int magic_number;
    const char *struct_name;
    are_equal_func are_equal;
    to_string_func to_string;
};

#define CONTAINABLE_MAGIC_NUMBER  0x1dae89c0

struct containable_struct {
    containable *containable_info;
    int other_elements;
};

containable *new_containable(const char *struct_name, are_equal_func are_equal, to_string_func to_string) {
    containable *c = malloc(sizeof(containable));
    c->magic_number = CONTAINABLE_MAGIC_NUMBER;
    c->struct_name = struct_name;
    c->are_equal = are_equal;
    c->to_string = to_string;
    return c;
}

static containable *get_containable(void *pointer) {
    if (pointer == NULL)
        return NULL;
    containable *c = ((struct containable_struct *)pointer)->containable_info;
    if (c == NULL || c->magic_number != CONTAINABLE_MAGIC_NUMBER)
        return NULL;
    return c;
}

bool is_containable_instance(void *pointer) {
    return get_containable(pointer) != NULL;
}

bool containables_are_equal(void *pointer_a, void *pointer_b) {
    containable *a = get_containable(pointer_a);
    containable *b = get_containable(pointer_b);
    if (a == NULL || b == NULL)
        return false;
    if (strcmp(a->struct_name, b->struct_name) != 0)
        return false;
    if (a->are_equal == NULL)
        return false;
    return a->are_equal(pointer_a, pointer_b);
}

const char *containable_to_string(void *pointer) {
    containable *c = get_containable(pointer);
    if (c == NULL || c->to_string == NULL)
        return NULL;
    return c->to_string(pointer);
}
