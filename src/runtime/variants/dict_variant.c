#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct dict_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    dict *dict;
} dict_instance;

static execution_outcome initialize(dict_instance *obj, variant *args, variant *named_args, exec_context *ctx) {
    // this dict shall contain variants
    obj->dict = new_dict(variant_item_info);
    return ok_outcome(NULL);
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

static execution_outcome get_element(dict_instance *obj, variant *index) {
    if (!variant_instance_of(index, str_type))
        return exception_outcome(new_exception_variant(
            "dict elements must be indexed by strings"));
        
    const char *key = str_variant_as_str(index);
    if (!dict_has(obj->dict, key))
        return exception_outcome(new_exception_variant(
            "key '%s' not found in dictionary", key));
    
    return ok_outcome(dict_get(obj->dict, key));
}

static execution_outcome set_element(dict_instance *obj, variant *index, variant *value) {
    if (!variant_instance_of(index, str_type))
        return exception_outcome(new_exception_variant(
            "dict elements must be indexed by strings"));
        
    const char *key = str_variant_as_str(index);
    
    if (!dict_has(obj->dict, key)) {
        dict_set(obj->dict, key, value);
        variant_inc_ref(value);
        return ok_outcome(NULL);
    }

    variant *old_value = dict_get(obj->dict, key);
    if (old_value == value)
        return ok_outcome(NULL);
    
    variant_drop_ref(old_value);
    dict_set(obj->dict, key, value);
    variant_inc_ref(value);
    return ok_outcome(NULL);
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
    .equality_checker = (equals_func)are_equal,
    .get_element = (get_element_func)get_element,
    .set_element = (set_element_func)set_element,
};

variant *new_dict_variant() {
    execution_outcome ex = variant_create(dict_type, NULL, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    return (variant *)ex.result;
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

variant *new_dict_variant_owning(dict *dict) {
    dict_instance *obj = (dict_instance *)new_dict_variant();
    obj->dict = dict;
    return (variant *)obj;
}

dict *dict_variant_as_dict(variant *v) {
    if (!variant_instance_of(v, dict_type))
        return NULL;
    return ((dict_instance *)v)->dict;
}

void dict_variant_set(variant *v, variant *key, variant *item) {
    if (!variant_instance_of(v, dict_type))
        return;
    dict_instance *obj = (dict_instance *)v;

    if (!variant_instance_of(key, str_type))
        return; // exception?

    dict_set(obj->dict, str_variant_as_str(key), item);
    variant_inc_ref(item);
}
