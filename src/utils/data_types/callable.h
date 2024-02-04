#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "variant.h"
#include "exec_context.h"
#include "../containers/_module.h"
#include "../containers/_module.h"

// We want to support both callables being called,
// with a list of variants as arguments
// returning a variant as result,

typedef struct callable callable;

typedef failable_variant callable_handler(
    list *positional_args,
    dict *named_args,
    void *callable_data, // used for AST nodes
    variant *this_obj,
    exec_context *ctx
);


callable *new_callable(const char *name, callable_handler *func, void *callable_data);

const char *callable_name(callable *c);
failable_variant callable_call(callable *c, list *positional_args, dict *named_args, variant *this_obj, exec_context *ctx);
bool callables_are_equal(callable *a, callable *b);
const char *callable_to_string(callable *c);

extern contained_item *containing_callables;

#endif
