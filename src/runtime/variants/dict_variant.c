#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct dict_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    dict *dict;
} dict_instance;

static void initialize(dict_instance *obj, variant *args, variant *named_args) {
    // this dict shall contain variants
    obj->dict = new_dict(variant_item_info);
}

static void destruct(dict_instance *obj) {
    // drop references for any contained items before we drop the dict
    for_dict(obj->dict, it, variant, key) {
        variant_drop_ref(key);
        variant_drop_ref((variant *)dict_get(obj->dict, str_variant_as_str(key)));
    }
    dict_free(obj->dict);
}

static variant *stringify(dict_instance *obj) {
    variant *key_separator = new_str_variant(": ");
    variant *entries_separator = new_str_variant(", ");
    variant *result = new_str_variant("");

    bool first = true;
    for_dict(obj->dict, it, variant, key) {
        if (!first)
            str_variant_append(result, entries_separator);

        str_variant_append(result, key);
        str_variant_append(result, key_separator);

        variant *item = dict_get(obj->dict, str_variant_as_str(key));
        variant *item_str = variant_to_string(item);
        str_variant_append(result, item_str);
        variant_drop_ref(item_str);
        first = false;
    }

    variant_drop_ref(key_separator);
    variant_drop_ref(entries_separator);
    return result;
}

static unsigned hash(dict_instance *obj) {
    // we could hash each item
}

static int compare(dict_instance *a, dict_instance *b) {
    
}

static bool are_equal(dict_instance *a, dict_instance *b) {
    return dicts_are_equal(a->dict, b->dict);
}

variant_type *dict_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "dict",
    .parent_type = NULL,
    .instance_size = sizeof(dict_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_dict_variant() {
    return (variant *)variant_create(dict_type, NULL, NULL);
}

variant *new_dict_variant_of(int entries_count, ...) {
    dict_instance *obj = (dict_instance *)new_dict_variant();
    va_list args;
    va_start(args, entries_count);
    while (entries_count-- > 0) {
        const char *key = va_arg(args, const char *);
        variant *item = va_arg(args, variant *);
        dict_set(obj->dict, key, item); // we cannot store variants as keys....
    }
    va_end(args);
    return (variant *)obj;
}

dict *dict_variant_as_dict(variant *v) {
    if (!variant_is(v, dict_type))
        return NULL;
    return ((dict_instance *)v)->dict;
}

void dict_variant_set(variant *v, variant *key, variant *item) {
    if (!variant_is(v, dict_type))
        return;
    dict_instance *obj = (dict_instance *)v;

    if (!variant_is(key, str_type))
        return; // exception?

    dict_set(obj->dict, str_variant_as_str(key), item);
    variant_inc_ref(item);
}
