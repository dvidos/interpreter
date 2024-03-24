#include <string.h>
#include "failable.h"
#include "cstr.h"
#include "../containers/_containers.h"



static bool cstrs_are_equal(cstr *a, cstr *b) {
    return strcmp(a, b) == 0;
}

static const void cstr_describe(cstr *s, str *str) {
    str_adds(str, s);
}

contained_item_info *cstr_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "cstr",
    .are_equal = (items_equal_func)cstrs_are_equal,
    .describe = (describe_item_func)cstr_describe
};


STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(cstr);
