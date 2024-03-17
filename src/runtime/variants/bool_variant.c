#include "_internal.h"
#include <string.h>
#include <stdio.h>


typedef struct bool_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    bool value;
} bool_instance;

static execution_outcome initialize(bool_instance *obj, variant *args, variant *named_args, exec_context *ctx) {
    obj->value = false;
    return ok_outcome(NULL);
}

static void destruct(bool_instance *obj) {
}

static void copy_initialize(bool_instance *obj, bool_instance *original) {
    obj->value = original->value;
}

static variant *stringify(bool_instance *obj) {
    return new_str_variant(obj->value ? "true" : "false");
}

static unsigned hash(bool_instance *obj) {
    return (unsigned)(obj->value);
}

static int compare(bool_instance *a, bool_instance *b) {
    return a->value - b->value;
}

static bool are_equal(bool_instance *a, bool_instance *b) {
    return a->value == b->value;
}

variant_type *bool_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "bool",
    .parent_type = NULL,
    .instance_size = sizeof(bool_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_bool_variant(bool value) {
    execution_outcome ex = variant_create(bool_type, NULL, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    bool_instance *i = (bool_instance *)ex.result;
    i->value = value;
    return (variant *)i;
}

bool bool_variant_as_bool(variant *v) {
    if (!variant_instance_of(v, bool_type))
        return 0;
    return ((bool_instance *)v)->value;
}
