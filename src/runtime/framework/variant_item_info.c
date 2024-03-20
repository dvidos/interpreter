#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant_item_info.h"
#include "../../utils/data_types/callable.h"


static const void item_info_variant_describe(variant *v, str_builder *sb) {
    variant *s = variant_to_string(v);
    str_builder_add(sb, str_variant_as_str(s));
    variant_drop_ref(s);
}

static bool item_info_are_equal(variant *a, variant *b) {
    return variants_are_equal(a, b);
}

contained_item_info *variant_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "variant",
    .are_equal = (items_equal_func)item_info_are_equal,
    .describe = (describe_item_func)item_info_variant_describe
};
