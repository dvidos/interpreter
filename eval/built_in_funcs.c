#include <stdlib.h>
#include <string.h>
#include "built_in_funcs.h"
#include "../utils/variant.h"
#include "../utils/callable.h"
#include "../utils/containers/list.h"

#define at_least(value, threshold)   ((value) >= (threshold) ? (value) : (threshold))
#define at_most(value, threshold)    ((value) <= (threshold) ? (value) : (threshold))
#define between(value, low, high)    at_most(at_least(value, low), high)

static dict *built_in_functions = NULL;


#define BUILT_IN_CALLABLE(name, ret_type, args_cnt, ...)  \
    static failable_variant name ## _func_body(list *arguments); \
    static inline callable *built_in_ ## name () { \
        return new_callable(#name, name ## _func_body, list_of(args_cnt, ## __VA_ARGS__), ret_type); \
    } \
    static failable_variant name ## _func_body(list *args)

#define STR_ARG(num)    variant_as_str(list_get(args, num))
#define INT_ARG(num)    variant_as_int(list_get(args, num))
#define RET_STR(val)    return ok_variant(new_str_variant(val))
#define RET_INT(val)    return ok_variant(new_int_variant(val))


BUILT_IN_CALLABLE(strlen, VT_INT, 1, VT_STR) {
    const char *str = STR_ARG(0);
    RET_INT(strlen(str));
}

BUILT_IN_CALLABLE(substr, VT_STR, 3, VT_STR, VT_INT, VT_INT) {
    const char *str = STR_ARG(0);
    int index = INT_ARG(1);
    int len = INT_ARG(2);

    int actual_index = index >= 0 ? index : strlen(str) - (-index);
    actual_index = between(actual_index, 0, strlen(str));

    int actual_len = len >= 0 ? len : strlen(str + actual_index) - (-len);
    actual_len = between(actual_len, 0, strlen(str + actual_index));

    char *p = malloc(actual_len + 1);
    memcpy(p, str + actual_index, actual_len);
    p[actual_len] = 0;

    RET_STR(p);
}

BUILT_IN_CALLABLE(strpos, VT_INT, 2, VT_STR, VT_STR, VT_INT) {
    const char *heystack = STR_ARG(0);
    const char *needle = STR_ARG(1);

    char *ptr = strstr(heystack, needle);
    int pos = (ptr == NULL) ? -1 : ptr - heystack;

    RET_INT(pos);
}



static inline void add_callable_to_funcs_table(callable *c) {
    dict_set(built_in_functions, callable_name(c), c);
}
void initialize_built_in_funcs_table() {
    built_in_functions = new_dict(16);

    add_callable_to_funcs_table(built_in_substr());
    add_callable_to_funcs_table(built_in_strpos());
    add_callable_to_funcs_table(built_in_strlen());
}

dict *get_built_in_funcs_table() {
    return built_in_functions;
}
