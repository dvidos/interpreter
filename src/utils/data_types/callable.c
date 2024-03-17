#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "callable.h"


/*
    callable is only used in built in functions,
    to discover a built-in, 
    or to call a built-in, by passing another callable as an argument
    maybe we don't need this intermediate class, as we need a "list" or "dict"

    but let's do this on another day!
*/


struct callable {
    item_info *item_info;
    const char *name;
    const char *description;
    callable_handler *handler;
    void *callable_data;
};

callable *new_callable(const char *name, callable_handler *handler, void *callable_data) {
    callable *c = malloc(sizeof(callable));
    c->item_info = callable_item_info;
    c->name = name;
    c->handler = handler;
    c->callable_data = callable_data;
    return c;
}

const char *callable_name(callable *c) {
    return c->name;
}

execution_outcome callable_call(callable *c, list *positional_args, variant *this_obj, exec_context *ctx) {
    if (ctx == NULL)
        return failed_outcome("callable_call(): execution context was not passed in");
    
    return c->handler(positional_args, c->callable_data, this_obj, ctx);
}

item_info *callable_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "callable",
    .are_equal = NULL,
    .describe = NULL,
};

