#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "../../runtime/execution/exec_context.h"
#include "../../runtime/variants/_module.h"
#include "../../containers/_module.h"

// We want to support both callables being called,
// with a list of variants as arguments
// returning a variant as result,

typedef struct callable callable;

typedef execution_outcome callable_handler(
    list *positional_args,
    void *callable_data, // used for AST nodes
    variant *this_obj,
    exec_context *ctx
);


callable *new_callable(const char *name, callable_handler *func, void *callable_data);

const char *callable_name(callable *c);
execution_outcome callable_call(callable *c, list *positional_args, variant *this_obj, exec_context *ctx);

extern item_info *callable_item_info;

#endif
