#include <string.h>
#include "failable.h"
#include "str.h"
#include "../containers/_containers.h"



contained_item_info *str_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "str",
    .are_equal = (items_equal_func)strs_are_equal,
    .describe = (describe_item_func)str_describe
};

extern contained_item_info *str_item_info;

bool strs_are_equal(str *a, str *b) {
    return strcmp(a, b) == 0;
}

const void str_describe(str *s, str_builder *sb) {
    str_builder_add(sb, s);
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(str);
