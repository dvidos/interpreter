#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "built_in_funcs.h"
#include "../../utils/mem.h"
#include "../../utils/execution_outcome.h"
#include "../../containers/_containers.h"
#include "../../utils/cstr.h"
#include "../../utils/str.h"
#include "../../utils/origin.h"
#include "../../utils/data_types/callable.h"
#include "../execution/exec_context.h"


#define at_least(value, threshold)   ((value) >= (threshold) ? (value) : (threshold))
#define at_most(value, threshold)    ((value) <= (threshold) ? (value) : (threshold))
#define between(value, low, high)    at_most(at_least(value, low), high)

static list *built_in_funcs_list = NULL;
static dict *built_in_funcs_dict = NULL;


#define HANDLER_ARGUMENTS \
    list *arg_values, \
    void *ast_node, \
    variant *this_obj, \
    dict *captured_values, \
    origin *call_origin, \
    exec_context *ctx


#define BUILT_IN(name)  \
        static execution_outcome built_in_ ## name ## _handler(HANDLER_ARGUMENTS); \
        static inline callable *make_callable_for_ ## name () \
            { return new_callable(#name, built_in_ ## name ## _handler, NULL, NULL, NULL); } \
        static execution_outcome built_in_ ## name ## _handler(HANDLER_ARGUMENTS)


#define STR_ARG(num)    (num >= list_length(arg_values)) ? NULL : str_variant_as_str(list_get(arg_values, num))
#define INT_ARG(num)    (num >= list_length(arg_values)) ? 0 : int_variant_as_int(list_get(arg_values, num))
#define LIST_ARG(num)   (num >= list_length(arg_values)) ? NULL : list_variant_as_list(list_get(arg_values, num))
#define DICT_ARG(num)   (num >= list_length(arg_values)) ? NULL : dict_variant_as_dict(list_get(arg_values, num))
#define CALL_ARG(num)   (num >= list_length(arg_values)) ? NULL : callable_variant_as_callable(list_get(arg_values, num))
#define VARNT_ARG(num)  (num >= list_length(arg_values)) ? NULL : list_get(arg_values, num)

#define RET_STR(val)    ok_outcome(new_str_variant(val))
#define RET_INT(val)    ok_outcome(new_int_variant(val))
#define RET_BOOL(val)   ok_outcome(new_bool_variant(val))
#define RET_VARNT(var)  ok_outcome(var)
#define RET_VOID()      ok_outcome(void_singleton)



BUILT_IN(new) {
    // first argument is type, rest are initialization args
    // e.g. new(str, 'hello') will be split into:
    //      - acquire and prepare a 'str' instance
    //      - call the type initializer with the 'hello' as first argument

    variant *v = VARNT_ARG(0);
    if (v == NULL || v->_type != type_of_types)
        return exception_outcome(new_exception_variant("new() requires a type as first argument"));
    variant_type *type = (variant_type *)v;
    
    // we need to create a new slice of the args list, excluding the first argument.
    list *args_slice = new_list(variant_item_info);
    for (int i = 1; i < list_length(arg_values); i++)
        list_add(args_slice, list_get(arg_values, i));
    variant *initializer_arg_values = new_list_variant_owning(args_slice);

    variant *instance = malloc(type->instance_size);
    instance->_type = type;
    instance->_references_count = 1;

    if (type->initializer != NULL) {
        execution_outcome ex;
        ex = type->initializer(instance, initializer_arg_values, ctx);
        if (ex.failed || ex.excepted) return ex;
    }

    // we should improve the "clone()", to allow us to copy on write.
    // shouldn't drop object I did not create.
    // variant_drop_ref(initializer_arg_values);

    return ok_outcome(instance);
}

BUILT_IN(type) {
    // first argument is an object, return it's type
    variant *a = VARNT_ARG(0);
    if (a == NULL)
        return RET_VARNT(void_singleton);
    
    return RET_VARNT((variant *)a->_type);
}




BUILT_IN(strlen) {
    const char *s = STR_ARG(0);
    return RET_INT(strlen(s));
}

BUILT_IN(substr) {
    const char *s = STR_ARG(0);
    int index = INT_ARG(1);
    int len = INT_ARG(2);

    int actual_index = index >= 0 ? index : strlen(s) - (-index);
    actual_index = between(actual_index, 0, strlen(s));

    int actual_len = len >= 0 ? len : strlen(s + actual_index) - (-len);
    actual_len = between(actual_len, 0, strlen(s + actual_index));

    char *p = malloc(actual_len + 1);
    memcpy(p, s + actual_index, actual_len);
    p[actual_len] = 0;

    return RET_STR(p);
}

BUILT_IN(strpos) {
    const char *heystack = STR_ARG(0);
    const char *needle = STR_ARG(1);

    char *ptr = strstr(heystack, needle);
    int pos = (ptr == NULL) ? -1 : ptr - heystack;

    return RET_INT(pos);
}

str *log_line_builder = NULL;

BUILT_IN(log) {
    int args_count = list_length(arg_values);

    if (log_line_builder == NULL)
        log_line_builder = new_str();

    str_clear(log_line_builder);
    for (int i = 0; i < args_count; i++) {
        variant *s = variant_to_string(list_get(arg_values, i));
        str_adds(log_line_builder, str_variant_as_str(s));
        variant_drop_ref(s);
        if (i < args_count - 1)
            str_addc(log_line_builder, ' ');
    }

    exec_context_log_line(str_cstr(log_line_builder));
    FILE *echo = exec_context_get_log_echo();
    if (echo != NULL)
        fprintf(echo, "%s\n", str_cstr(log_line_builder));

    return RET_VOID();
}
BUILT_IN(input) {
    // something like gets() ?
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return exception_outcome(new_exception_variant(
            "EOF while reading input"));
    }

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        

    char *p = malloc(strlen(buffer) + 1);
    strcpy(p, buffer);
    return RET_STR(p);
}
BUILT_IN(output) {
    int args_count = list_length(arg_values);
    str *str = new_str();
    for (int i = 0; i < args_count; i++) {
        variant *s = variant_to_string(list_get(arg_values, i));
        str_adds(str, str_variant_as_str(s));
        variant_drop_ref(s);
        if (i < args_count - 1)
            str_addc(str, ' ');
    }
    str_addc(str, '\n');
    fputs(str_cstr(str), stdout);
    str_free(str);
    return RET_VOID();
}

BUILT_IN(srand) {
    int seed = INT_ARG(0);
    srand(seed == 0 ? time(NULL) : seed);
    return RET_VOID();
}

BUILT_IN(rand) {
    return RET_INT(rand());
}

BUILT_IN(str) {
    variant *v = list_get(arg_values, 0);
    return RET_VARNT(variant_to_string(v));
}
BUILT_IN(int) {
    variant *v = list_get(arg_values, 0);
    if (variant_instance_of(v, bool_type)) {
        return RET_INT(bool_variant_as_bool(v) ? 1 : 0);
    } else {
        int i = atoi(str_variant_as_str(variant_to_string(v)));
        return RET_INT(i);
    }
}
BUILT_IN(bool) {
    variant *v = list_get(arg_values, 0);
    if (variant_instance_of(v, void_type)) {
        return RET_VARNT(false_instance);
    } else if (variant_instance_of(v, int_type)) {
        return RET_VARNT(int_variant_as_int(v) == 0 ? false_instance : true_instance);
    } else {
        const char *s = str_variant_as_str(variant_to_string(v));
        // most strings evaluate to true, only empty, "0" and "false" evaluate to false
        bool is_false = (strlen(s) == 0 || strcmp(s, "0") == 0 || strcmp(s, "false") == 0);
        return RET_BOOL(!is_false);
    }
}




static inline void add_callable(callable *c) {
    list_add(built_in_funcs_list, c);
    dict_set(built_in_funcs_dict, callable_name(c), c);
}

void initialize_built_in_funcs_table() {
    built_in_funcs_list = new_list(callable_item_info);
    built_in_funcs_dict = new_dict(callable_item_info);

    add_callable(make_callable_for_new());
    add_callable(make_callable_for_type());
    add_callable(make_callable_for_substr());
    add_callable(make_callable_for_strpos());
    add_callable(make_callable_for_strlen());
    add_callable(make_callable_for_log());
    add_callable(make_callable_for_input());
    add_callable(make_callable_for_output());
    add_callable(make_callable_for_rand());
    add_callable(make_callable_for_srand());
    add_callable(make_callable_for_str());
    add_callable(make_callable_for_int());
    add_callable(make_callable_for_bool());
}

dict *get_built_in_funcs_table() {
    return built_in_funcs_dict;
}
