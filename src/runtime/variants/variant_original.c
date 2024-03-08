#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant_original.h"
#include "../../utils/data_types/callable.h"

#include "int_variant.h"
#include "str_variant.h"
#include "bool_variant.h"
#include "float_variant.h"



bool variants_are_equal(variant *a, variant *b);
const void variant_describe(variant *v, str_builder *sb);

item_info *variant_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "variant",
    .are_equal = (items_equal_func)variants_are_equal,
    .describe = (describe_item_func)variant_describe
};

const char *variant_as_str(variant *v) {
    // we should not return the char here, we should return the str_variant,
    // so that caller can drop the reference, when they are done.
    if (variant_is(v, void_type)) {
        return "(void)";
    } else if (variant_is(v, str_type)) {
        return str_variant_as_str(v);
    } else if (variant_is(v, int_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, bool_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, float_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, list_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, dict_type)) {
        return str_variant_as_str(variant_to_string(v));
    }

    return NULL;
}


bool variants_are_equal(variant *a, variant *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;

    if (variant_is(a, void_type) && variant_is(b, void_type)) {
        return true;
    } else if (variant_is(a, str_type) && variant_is(b, str_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, int_type) && variant_is(b, int_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, bool_type) && variant_is(b, bool_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, float_type) && variant_is(b, float_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, list_type) && variant_is(b, list_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, dict_type) && variant_is(b, dict_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, callable_type) && variant_is(b, callable_type)) {
        return a->_type->equality_checker(a, b);
    }

    return false;
}

const void variant_describe(variant *v, str_builder *sb) {
    str_builder_add(sb, variant_as_str(v));
}
