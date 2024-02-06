#include "class.h"


class *get_class(void *instance) {
    if (instance == NULL)
        return NULL;
    // the idea is that the first poiner of the instance points to the class strung
    class *class_ptr = (class *)(*instance);
    if (class_ptr->classdef_magic != CLASSDEF_MAGIC)
        return NULL;
    return class_ptr;
}

bool is_instance(void *instance, class *c) {
    return get_class(instance) == c;
}

const char *instance_describe(void *instance) {
    class *c = get_class(instance);
    if (c == NULL)
        return NULL;
    return c->to_string(instance);
}

bool instances_are_equal(void *a, void *b) {
    class *c = get_class(a);
    if (c == NULL)
        return a == b;
    
    if (c != get_class(b))
        return false;
    return c->are_equal(a, b);
}

