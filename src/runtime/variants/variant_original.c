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


static const void item_info_variant_describe(variant *v, str_builder *sb) {
    variant *s = variant_to_string(v);
    str_builder_add(sb, str_variant_as_str(s));
    variant_drop_ref(s);
}

static bool item_info_are_equal(variant *a, variant *b) {
    return variants_are_equal(a, b);
}

const char *deprecated_variant_as_const_char(variant *v) {
    // we should not return the char here, we should return the str_variant,
    // so that caller can drop the reference, when they are done.
    if (variant_instance_of(v, void_type)) {
        return "(void)";
    } else if (variant_instance_of(v, str_type)) {
        return str_variant_as_str(v);
    } else if (variant_instance_of(v, int_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_instance_of(v, bool_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_instance_of(v, float_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_instance_of(v, list_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_instance_of(v, dict_type)) {
        return str_variant_as_str(variant_to_string(v));
    }

    return NULL;
}

item_info *variant_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "variant",
    .are_equal = (items_equal_func)item_info_are_equal,
    .describe = (describe_item_func)item_info_variant_describe
};
