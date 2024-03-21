#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "callable.h"
#include "../../containers/_containers.h"


/*
    callable is only used in built in functions,
    to discover a built-in, 
    or to call a built-in, by passing another callable as an argument
    maybe we don't need this intermediate class, as we need a "list" or "dict"

    but let's do this on another day!
*/


struct callable {
    contained_item_info *item_info;
    const char *name;
    callable_handler *handler;
    void *callable_data;       // used for AST nodes
    variant *this_obj;         // optional for object methods
    dict *captured_values;     // optional for closures
};

callable *new_callable(
    const char *name,
    callable_handler *handler,
    void *callable_data,
    variant *this_obj,
    dict *captured_values
) {
    callable *c = malloc(sizeof(callable));
    c->item_info = callable_item_info;
    c->name = name;
    c->handler = handler;
    c->callable_data = callable_data;
    c->this_obj = this_obj;
    c->captured_values = captured_values;
    return c;
}

const char *callable_name(callable *c) {
    return c->name;
}

execution_outcome callable_call(
    callable *c,
    list *arg_values,
    variant *this_obj,
    origin *call_origin,
    exec_context *ctx
) {
    if (ctx == NULL)
        return failed_outcome("callable_call(): execution context was not passed in");
    
    if (this_obj == NULL && c->this_obj != NULL)
        this_obj = c->this_obj;

    return c->handler(
        arg_values,
        c->callable_data,
        this_obj,
        c->captured_values,
        call_origin,
        ctx
    );
}


static void describe_callable(void *pointer, str *str) {
    callable *c = (callable *)pointer;
    str_addf(str, "%s()", c->name);
}

contained_item_info *callable_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "callable",
    .are_equal = NULL,
    .describe = describe_callable,
};
