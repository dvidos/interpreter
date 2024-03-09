#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct list_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    list *list;
} list_instance;

static void initialize(list_instance *obj, variant *args, variant *named_args) {
    // this list shall contain variants
    obj->list = new_list(variant_item_info);
}

static void destruct(list_instance *obj) {
    // drop references for any contained items before we drop the list
    for_list(obj->list, it, variant, item) {
        variant_drop_ref(item);
    }
    list_free(obj->list);
}

static variant *stringify(list_instance *obj) {
    variant *separator = new_str_variant(", ");
    variant *result = new_str_variant("");

    bool first = true;
    for_list(obj->list, it, variant, item) {
        if (!first)
            str_variant_append(result, separator);

        variant *item_str = variant_to_string(item);
        str_variant_append(result, item_str);
        variant_drop_ref(item_str);
        first = false;
    }

    variant_drop_ref(separator);
    return result;
}

static unsigned hash(list_instance *obj) {
    // we could hash each item
}

static int compare(list_instance *a, list_instance *b) {
    
}

static bool are_equal(list_instance *a, list_instance *b) {
    return lists_are_equal(a->list, b->list);
}

static execution_outcome get_element(list_instance *obj, variant *index) {
    if (!variant_instance_of(index, int_type))
        return exception_outcome(new_exception_variant(
            "list elements must be indexed by integers"));
        
    int i = int_variant_as_int(index);
    if (i < 0 || i >= list_length(obj->list))
        return exception_outcome(new_exception_variant(
            "index %d outside of list bounds (%d..%d)", i, 0, list_length(obj->list) - 1));
    
    return ok_outcome(list_get(obj->list, i));
}

static execution_outcome set_element(list_instance *obj, variant *index, variant *value) {
    if (!variant_instance_of(index, int_type))
        return exception_outcome(new_exception_variant(
            "list elements must be indexed by integers"));
        
    int i = int_variant_as_int(index);
    if (i < 0 || i > list_length(obj->list))
        return exception_outcome(new_exception_variant(
            "index %d outside of list bounds (%d..%d)", i, 0, list_length(obj->list)));
    
    if (i == list_length(obj->list)) {
        list_add(obj->list, value);
        variant_inc_ref(value);
        return ok_outcome(NULL);
    }

    variant *old_value = list_get(obj->list, i);
    if (old_value == value)
        return ok_outcome(NULL);

    variant_drop_ref(old_value);
    list_set(obj->list, i, value);
    variant_inc_ref(value);
    return ok_outcome(NULL);
}

variant_type *list_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "list",
    .parent_type = NULL,
    .instance_size = sizeof(list_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal,
    .get_element = (get_element_func)get_element,
    .set_element = (set_element_func)set_element,
};

variant *new_list_variant() {
    return (variant *)variant_create(list_type, NULL, NULL);
}

variant *new_list_variant_of(int argc, ...) {
    list_instance *l = (list_instance *)new_list_variant();
    va_list args;
    va_start(args, argc);
    while (argc-- > 0) {
        variant *item = va_arg(args, variant *);
        list_add(l->list, item);
    }
    va_end(args);
    return (variant *)l;
}

list *list_variant_as_list(variant *v) {
    if (!variant_instance_of(v, list_type))
        return NULL;
    return ((list_instance *)v)->list;
}

void list_variant_append(variant *v, variant *item) {
    if (!variant_instance_of(v, list_type))
        return;
    list_instance *li = (list_instance *)v;

    list_add(li->list, item);
    variant_inc_ref(item);
}
