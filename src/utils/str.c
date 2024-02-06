#include <string.h>
#include "failable.h"
#include "str.h"
#include "containers/_module.h"



contained_item *containing_strs = &(contained_item){
    .type_name = "str",
    .are_equal = (are_equal_func)strs_are_equal,
    .to_string = (describe_func)str_describe
};

extern contained_item *containing_strs;

bool strs_are_equal(str *a, str *b) {
    return strcmp(a, b) == 0;
}

const void str_describe(str *s, str_builder *sb) {
    str_builder_add(sb, s);
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(str);
