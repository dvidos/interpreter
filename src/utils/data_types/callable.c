#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "callable.h"

class *callable_class = &(class){
    .type_name = "callable",
    .are_equal = (are_equal_func)callables_are_equal,
    .describe = (describe_func)callable_describe
};

struct callable {
    class *class;
    const char *name;
    const char *description;
    callable_handler *handler;
    void *callable_data;
};

callable *new_callable(const char *name, callable_handler *handler, void *callable_data) {
    callable *c = malloc(sizeof(callable));
    c->class = callable_class;
    c->name = name;
    c->handler = handler;
    c->callable_data = callable_data;
    return c;
}

const char *callable_name(callable *c) {
    return c->name;
}

bool callables_are_equal(callable *a, callable *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;

    if (strcmp(a->name, b->name) != 0)
        return false;
    if (a->handler != b->handler)
        return false;
    
    return true;
}

const void callable_describe(callable *c, str_builder *sb) {
    str_builder_addf(sb, "callable(@x%p)", c->handler);
}

failable_variant callable_call(callable *c, list *positional_args, dict *named_args, variant *this_obj, exec_context *ctx) {
    if (ctx == NULL)
        return failed_variant(NULL, "Execution context was not passed in");
    
    return c->handler(positional_args, named_args, c->callable_data, this_obj, ctx);
}
