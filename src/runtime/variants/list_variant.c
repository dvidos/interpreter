#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct list_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    list *list;
} list_instance;

static void initialize(list_instance *obj, variant *args, variant *named_args) {
    obj->list = new_list(variants_types);
}

static void destruct(list_instance *obj) {
    // drop references for any contained items before we drop the list
    for_list(obj->list, it, variant, item) {
        variant_drop_ref(item);
    }
    list_free(obj->list);
}

static void copy_initialize(list_instance *obj, list_instance *original) {
    // we need to duplicate the list, but should we clone the items?
}

static variant *stringify(list_instance *obj) {
    // how about separators? ", " or "\n" ???
}

static unsigned hash(list_instance *obj) {
    // we could hash each item
}

static int compare(list_instance *a, list_instance *b) {
    // ...
}

static bool are_equal(list_instance *a, list_instance *b) {
    // ...
}

variant_type *list_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "list",
    .parent_type = NULL,
    .instance_size = sizeof(list_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_list_variant() {
    list_instance *l = (list_instance *)variant_create(list_type, NULL, NULL);
    // ...
    return (variant *)l;
}

list *list_variant_as_list(variant *v) {
    if (!variant_is(v, list_type))
        return NULL;
    return ((list_instance *)v)->list;
}
