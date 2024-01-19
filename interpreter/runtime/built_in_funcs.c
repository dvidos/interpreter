#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "built_in_funcs.h"
#include "exec_context.h"
#include "../../utils/variant.h"
#include "../../utils/str_builder.h"
#include "../../utils/callable.h"
#include "../../utils/containers/_module.h"

#define at_least(value, threshold)   ((value) >= (threshold) ? (value) : (threshold))
#define at_most(value, threshold)    ((value) <= (threshold) ? (value) : (threshold))
#define between(value, low, high)    at_most(at_least(value, low), high)

static list *built_in_funcs_list = NULL;
static dict *built_in_funcs_dict = NULL;

#define BUILT_IN_CALLABLE(description, name, ret_type, var_args, args_cnt, ...)  \
    static failable_variant built_in_ ## name ## _body(list *arguments); \
    static inline callable *built_in_ ## name ## _callable() { \
        return new_callable(#name, description, built_in_ ## name ## _body, ret_type, list_of(NULL, args_cnt, ## __VA_ARGS__), var_args); \
    } \
    static failable_variant built_in_ ## name ## _body(list *args)

#define STR_ARG(num)    variant_as_str(list_get(args, num))
#define INT_ARG(num)    variant_as_int(list_get(args, num))
#define RET_STR(val)    ok_variant(new_str_variant(val))
#define RET_INT(val)    ok_variant(new_int_variant(val))
#define RET_VOID()      ok_variant(new_null_variant())


BUILT_IN_CALLABLE("int strlen(char *str);", 
                    strlen, VT_INT, false, 1, VT_STR) {
    const char *str = STR_ARG(0);
    return RET_INT(strlen(str));
}

BUILT_IN_CALLABLE("char *substr(char *substr, int start, int len);", 
                    substr, VT_STR, false, 3, VT_STR, VT_INT, VT_INT) {
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

    return RET_STR(p);
}

BUILT_IN_CALLABLE("int strpos(char *heystack, char *needle);",
                    strpos, VT_INT, false, 2, VT_STR, VT_STR) {
    const char *heystack = STR_ARG(0);
    const char *needle = STR_ARG(1);

    char *ptr = strstr(heystack, needle);
    int pos = (ptr == NULL) ? -1 : ptr - heystack;

    return RET_INT(pos);
}

BUILT_IN_CALLABLE("char *getenv(char *name);", getenv, VT_STR, false, 1, VT_STR) {
    const char *name = STR_ARG(0);
    return RET_STR(getenv(name));
}

BUILT_IN_CALLABLE("void log(anything, ...);", log, VT_VOID, true, 0) {
    int args_count = list_length(args);

    for (int i = 0; i < args_count; i++) {
        exec_context_log_str(variant_to_string(list_get(args, i)));
        if (i < args_count - 1)
            exec_context_log_str(" ");
    }
    exec_context_log_str("\n");

    return RET_VOID();
}

static inline void add_callable(callable *c) {
    list_add(built_in_funcs_list, c);
    dict_set(built_in_funcs_dict, callable_name(c), c);
}

void initialize_built_in_funcs_table() {
    built_in_funcs_list = new_list(containing_callables);
    built_in_funcs_dict = new_dict(containing_callables, 16);

    add_callable(built_in_substr_callable());
    add_callable(built_in_strpos_callable());
    add_callable(built_in_strlen_callable());
    add_callable(built_in_getenv_callable());
    add_callable(built_in_log_callable());
}

dict *get_built_in_funcs_table() {
    return built_in_funcs_dict;
}

void print_built_in_funcs_list() {
    for_list(built_in_funcs_list, it, callable, c) {
        printf("  %s\n", callable_description(c));
    }
}
