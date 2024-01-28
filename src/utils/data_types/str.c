#include <string.h>
#include "../failable.h"
#include "str.h"
#include "../containers/_module.h"



contained_item *containing_strs = &(contained_item){
    .type_name = "str",
    .are_equal = (are_equal_func)strs_are_equal,
    .to_string = (to_string_func)str_to_string
};

extern contained_item *containing_strs;

bool strs_are_equal(str *a, str *b) {
    return strcmp(a, b) == 0;
}

const char *str_to_string(str *s) {
    return (const char *)s;
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(str);
