#include "_internal.h"
#include <string.h>
#include <stdio.h>


typedef struct int_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    int value;
} int_instance;

static execution_outcome initialize(int_instance *obj, variant *args, exec_context *ctx) {
    obj->value = 0;
    return ok_outcome(NULL);
}

static void destruct(int_instance *obj) {
}

static void copy_initialize(int_instance *obj, int_instance *original) {
    obj->value = original->value;
}

static variant *stringify(int_instance *obj) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%d", obj->value);
    return new_str_variant(buffer);
}

static unsigned hash(int_instance *obj) {
    return (unsigned)(obj->value);
}

static int compare(int_instance *a, int_instance *b) {
    return a->value - b->value;
}

static bool are_equal(int_instance *a, int_instance *b) {
    return a->value == b->value;
}

variant_type *int_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "int",
    .parent_type = NULL,
    .instance_size = sizeof(int_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_int_variant(int value) {
    execution_outcome ex = variant_create(int_type, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    int_instance *i = (int_instance *)ex.result;
    i->value = value;
    return (variant *)i;
}

int int_variant_as_int(variant *v) {
    if (!variant_instance_of(v, int_type))
        return 0;
    return ((int_instance *)v)->value;
}
