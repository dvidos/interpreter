#include <stdlib.h>
#include <string.h>
#include "callable.h"

contained_item *containing_callables = &(contained_item){
    .type_name = "callable",
    .are_equal = (are_equal_func)callables_are_equal,
    .to_string = (to_string_func)callable_description
};

struct callable {
    const char *name;
    const char *description;
    callable_handler *handler;
    variant_type ret_type;
    list *arg_types;
    bool variadic;
    void *callable_data;
};

callable *new_callable(const char *name, const char *description, callable_handler *handler, variant_type ret_type, list *arg_types, bool variadic, void *callable_data) {
    callable *c = malloc(sizeof(callable));
    c->name = name;
    c->description = description;
    c->handler = handler;
    c->ret_type = ret_type;
    c->arg_types = arg_types;
    c->variadic = variadic;
    c->callable_data = callable_data;
    return c;
}

const char *callable_name(callable *c) {
    return c->name;
}
const char *callable_description(callable *c) {
    return c->description;
}

bool callables_are_equal(callable *a, callable *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;

    if (strcmp(a->name, b->name) != 0)
        return false;
    if (strcmp(a->description, b->description) != 0)
        return false;
    if (list_length(a->arg_types) != list_length(b->arg_types))
        return false;
    if (a->ret_type != b->ret_type)
        return false;
    if (a->handler != b->handler)
        return false;
    
    return true;
}


failable_variant callable_call(callable *c, list *positional_args, dict *named_args, void *call_data, variant *this_obj) {
    // assume we verify argument types.
    if (!c->variadic) {
        if (list_length(c->arg_types) != list_length(positional_args))
            return failed_variant(NULL, "%s() expected %d positional_args, got %d", c->name, list_length(c->arg_types), list_length(positional_args));

        for (int i = 0; i < list_length(positional_args); i++) {
            variant_type expected_type = (variant_type)list_get(c->arg_types, i);
            if (expected_type == VT_ANYTHING)
                continue;
            variant_type given_type = variant_get_type(list_get(positional_args, i));
            if (given_type != expected_type)
                return failed_variant(NULL, "argument #%d expected type %d, got %d", i, expected_type, given_type);
        }
    }
    
    return c->handler(positional_args, named_args, c->callable_data, call_data, this_obj);
}
