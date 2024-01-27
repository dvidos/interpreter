#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "built_in_funcs.h"
#include "exec_context.h"
#include "../../utils/containers/_module.h"
#include "../../utils/data_types/_module.h"
#include "../../utils/str_builder.h"

#define at_least(value, threshold)   ((value) >= (threshold) ? (value) : (threshold))
#define at_most(value, threshold)    ((value) <= (threshold) ? (value) : (threshold))
#define between(value, low, high)    at_most(at_least(value, low), high)

static list *built_in_funcs_list = NULL;
static dict *built_in_funcs_dict = NULL;

#define BUILT_IN_CALLABLE(description, name, ret_type, variadic, args_cnt, ...)  \
    static failable_variant built_in_ ## name ## _body(list *positional_args, dict *named_args, void *callable_data, void *call_data, variant *this_obj); \
    static inline callable *built_in_ ## name ## _callable() { \
        return new_callable(#name, description, built_in_ ## name ## _body, ret_type, list_of(NULL, args_cnt, ## __VA_ARGS__), variadic, NULL); \
    } \
    static failable_variant built_in_ ## name ## _body(list *positional_args, dict *named_args, void *callable_data, void *call_data, variant *this_obj)

#define STR_ARG(num)    variant_as_str(list_get(positional_args, num))
#define INT_ARG(num)    variant_as_int(list_get(positional_args, num))
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

str_builder *log_line_builder = NULL;

BUILT_IN_CALLABLE("void log(anything, ...);", log, VT_VOID, true, 0) {
    int args_count = list_length(positional_args);

    if (log_line_builder == NULL)
        log_line_builder = new_str_builder();

    str_builder_clear(log_line_builder);
    for (int i = 0; i < args_count; i++) {
        str_builder_cat(log_line_builder, variant_to_string(list_get(positional_args, i)));
        if (i < args_count - 1)
            str_builder_catc(log_line_builder, ' ');
    }

    exec_context_log_line(str_builder_charptr(log_line_builder));
    FILE *echo = exec_context_get_log_echo();
    if (echo != NULL)
        fprintf(echo, "%s\n", str_builder_charptr(log_line_builder));

    return RET_VOID();
}
BUILT_IN_CALLABLE("input(); -> str", input, VT_STR, false, 0) {
    // something like gets() ?
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return failed_variant(NULL, "EOF while reading input");

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        

    char *p = malloc(strlen(buffer) + 1);
    strcpy(p, buffer);
    return RET_STR(p);
}
BUILT_IN_CALLABLE("output(<anything>);", output, VT_NULL, true, 0) {
    int args_count = list_length(positional_args);
    str_builder *sb = new_str_builder();
    for (int i = 0; i < args_count; i++) {
        str_builder_cat(sb, variant_to_string(list_get(positional_args, i)));
        if (i < args_count - 1)
            str_builder_catc(sb, ' ');
    }
    str_builder_catc(sb, '\n');
    fputs(str_builder_charptr(sb), stdout);
    return RET_VOID();
}

BUILT_IN_CALLABLE("srand(int);", srand, VT_NULL, false, 1, VT_INT) {
    int seed = INT_ARG(0);
    srand(seed == 0 ? time(NULL) : seed);
    return RET_VOID();
}

BUILT_IN_CALLABLE("rand(); -> int", rand, VT_INT, false, 0) {
    return RET_INT(rand());
}

BUILT_IN_CALLABLE("str(<anything>); -> str", str, VT_STR, false, 1, VT_ANYTHING) {
    variant *v = list_get(positional_args, 0);
    str *s = variant_to_string(v);
    return RET_STR(s);
}
BUILT_IN_CALLABLE("int(<str>) -> int", int, VT_INT, false, 1, VT_STR) {
    variant *v = list_get(positional_args, 0);
    int i = variant_as_int(v);
    return RET_INT(i);
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
    add_callable(built_in_input_callable());
    add_callable(built_in_output_callable());
    add_callable(built_in_rand_callable());
    add_callable(built_in_srand_callable());
    add_callable(built_in_str_callable());
    add_callable(built_in_int_callable());
}

dict *get_built_in_funcs_table() {
    return built_in_funcs_dict;
}

void print_built_in_funcs_list() {
    for_list(built_in_funcs_list, it, callable, c) {
        printf("  %s\n", callable_description(c));
    }
}
