#include <string.h>
#include "failable.h"
#include "str.h"
#include "../containers/_module.h"



item_info *str_class = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "str",
    .are_equal = (items_equal_func)strs_are_equal,
    .describe = (describe_item_func)str_describe
};

extern item_info *str_class;

bool strs_are_equal(str *a, str *b) {
    return strcmp(a, b) == 0;
}

const void str_describe(str *s, str_builder *sb) {
    str_builder_add(sb, s);
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(str);
