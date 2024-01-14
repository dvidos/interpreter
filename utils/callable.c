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
    callable_func *func;
    list *arg_types;
    variant_type ret_type;
};

callable *new_callable(const char *name, const char *description, callable_func *func, list *arg_types, variant_type ret_type) {
    callable *c = malloc(sizeof(callable));
    c->name = name;
    c->description = description;
    c->func = func;
    c->arg_types = arg_types;
    c->ret_type = ret_type;
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
    
    return true;
}


failable_variant callable_call(callable *c, list *arguments) {
    // assume we verify argument types.
    if (list_length(c->arg_types) != list_length(arguments))
        return failed_variant("expected %d arguments, got %d", list_length(c->arg_types), list_length(arguments));
    for (int i = 0; i < list_length(arguments); i++) {
        variant_type expected_type = (variant_type)list_get(c->arg_types, i);
        variant_type given_type = variant_get_type(list_get(arguments, i));
        if (given_type != expected_type)
            return failed_variant("argument #%d expected type %d, got %d", i, expected_type, given_type);
    }
    
    return c->func(arguments);
}
