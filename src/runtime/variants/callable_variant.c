#include "_internal.h"
#include "../../utils/hash.h"
#include "../../utils/data_types/callable.h"
#include <string.h>
#include <stdio.h>


// the callable class must be callable by design,
// it will be used for base functions, class methods (bound to 'this'),
// even anonymous functions (bound to variables at the time of evaluation)

typedef struct callable_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    callable *callable;
    variant *member_parent; // reference, not owned value, can be null
} callable_instance;

static void initialize(callable_instance *obj, variant *args, variant *named_args) {
    obj->callable = NULL;
    obj->member_parent = NULL;
}

static void destruct(callable_instance *obj) {
}

static variant *stringify(callable_instance *obj) {
    if (obj->callable == NULL)
        return NULL;
    if (obj->member_parent != NULL) {
        return new_str_variant("%s.%s()", 
            obj->member_parent->_type->name,
            callable_name(obj->callable));
    } else {
        return new_str_variant("%s()", 
            callable_name(obj->callable));
    }
}

static unsigned hash(callable_instance *obj) {
    // we could hash each item
}

static int compare(callable_instance *a, callable_instance *b) {
    
}

static bool are_equal(callable_instance *a, callable_instance *b) {
    return callables_are_equal(a->callable, b->callable);
}

variant_type *callable_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "callable",
    .parent_type = NULL,
    .instance_size = sizeof(callable_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_callable_variant(callable *c, variant *member_parent) {
    callable_instance *obj = (callable_instance *)variant_create(callable_type, NULL, NULL);
    obj->callable = c;
    obj->member_parent = member_parent;
    return (variant *)obj;
}

callable *callable_variant_as_callable(variant *v) {
    if (!variant_instance_of(v, callable_type))
        return NULL;
    return ((callable_instance *)v)->callable;
}
