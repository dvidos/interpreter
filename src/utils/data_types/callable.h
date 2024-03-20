#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "../../runtime/execution/exec_context.h"
#include "../../runtime/variants/_variants.h"
#include "../../containers/_containers.h"


typedef struct callable callable;


// the C call handler function and it's arguments
typedef execution_outcome callable_handler(
    list *arg_values,
    void *ast_node, // used for AST nodes
    variant *this_obj,
    dict *captured_values, // optional for closures
    origin *call_origin, // source of call
    exec_context *ctx
);

// defined at callable creation time
callable *new_callable(
    const char *name,
    callable_handler *func,
    void *ast_node,   // used for AST nodes
    variant *this_obj,     // optional early binding
    dict *captured_values // optional for closures
);

const char *callable_name(callable *c);

// passed in at callable call time
execution_outcome callable_call(
    callable *c, 
    list *arg_values,  // each item a variant
    variant *this_obj, // late binding,
    origin *call_origin,
    exec_context *ctx
);

extern contained_item_info *callable_item_info;

#endif
