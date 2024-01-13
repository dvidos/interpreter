#include <stdlib.h>
#include "callable.h"

struct callable {
    const char *name;
    callable_func *func;
    list *arg_types;
    variant_type ret_type;
};

callable *new_callable(const char *name, callable_func *func, list *arg_types, variant_type ret_type) {
    callable *c = malloc(sizeof(callable));
    c->name = name;
    c->func = func;
    c->arg_types = arg_types;
    c->ret_type = ret_type;
    return c;
}

const char *callable_name(callable *c) {
    return c->name;
}

failable_variant callable_call(callable *c, list *arguments) {
    // assume we verify argument types.
    if (list_length(c->arg_types) != list_length(arguments))
        return failed("expected %d arguments, got %d", list_length(c->arg_types), list_length(arguments));
    for (int i = 0; i < list_length(arguments); i++) {
        variant_type expected_type = (variant_type)list_get(c->arg_types, i);
        variant_type given_type = variant_get_type(list_get(arguments, i));
        if (given_type != expected_type)
            return failed("argument #%d expected type %d, got %d", i, expected_type, given_type);
    }
    
    return c->func(arguments);
}
