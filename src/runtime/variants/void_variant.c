#include "_internal.h"
#include <string.h>
#include <stdio.h>


typedef struct void_variant_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
} void_variant_instance;

static void initialize(void_variant_instance *obj, variant *args, variant *named_args) {
}

static void destruct(void_variant_instance *obj) {
}

static void copy_initialize(void_variant_instance *obj, void_variant_instance *original) {
}

static variant *stringify(void_variant_instance *obj) {
    return new_str_variant("void");
}

static unsigned hash(void_variant_instance *obj) {
    return 0;
}

static int compare(void_variant_instance *a, void_variant_instance *b) {
    return 0;
}

static bool are_equal(void_variant_instance *a, void_variant_instance *b) {
    return true;
}

variant_type *void_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "void",
    .parent_type = NULL,
    .instance_size = sizeof(void_variant_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_void_variant() {
    return variant_create(void_type, NULL, NULL);
}
