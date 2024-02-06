#include <string.h>
#include "failable.h"
#include "str.h"
#include "containers/_module.h"



class *str_class = &(class){
    .classdef_magic = CLASSDEF_MAGIC,
    .type_name = "str",
    .are_equal = (are_equal_func)strs_are_equal,
    .describe = (describe_func)str_describe
};

extern class *str_class;

bool strs_are_equal(str *a, str *b) {
    return strcmp(a, b) == 0;
}

const void str_describe(str *s, str_builder *sb) {
    str_builder_add(sb, s);
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(str);
