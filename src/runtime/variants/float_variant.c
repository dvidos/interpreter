#include "_internal.h"
#include <string.h>
#include <stdio.h>
#include <float.h>


typedef struct float_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    float value;
} float_instance;

static execution_outcome initialize(float_instance *obj, variant *args, exec_context *ctx) {
    obj->value = 0.0;
    return ok_outcome(NULL);
}

static void destruct(float_instance *obj) {
}

static void copy_initialize(float_instance *obj, float_instance *original) {
    obj->value = original->value;
}

static variant *stringify(float_instance *obj) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%f", obj->value);
    return new_str_variant(buffer);
}

static unsigned hash(float_instance *obj) {
    return (unsigned)(obj->value);
}

static int compare(float_instance *a, float_instance *b) {
    return (a > b) ? 1 : (a < b ? -1 : 0);
}

static bool are_equal(float_instance *a, float_instance *b) {
    return (a > b) ? ((a - b) < FLT_EPSILON) : ((b - a) < FLT_EPSILON);
}

variant_type *float_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "float",
    .parent_type = NULL,
    .instance_size = sizeof(float_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_float_variant(float value) {
    execution_outcome ex = variant_create(float_type, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    float_instance *i = (float_instance *)ex.result;
    i->value = value;
    return (variant *)i;
}

float float_variant_as_float(variant *v) {
    if (!variant_instance_of(v, float_type))
        return 0;
    return ((float_instance *)v)->value;
}
