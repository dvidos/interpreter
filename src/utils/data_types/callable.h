#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "exec_context.h"
#include "../../runtime/variants/variant.h"
#include "../../runtime/execution/execution_outcome.h"
#include "../../containers/_module.h"

// We want to support both callables being called,
// with a list of variants as arguments
// returning a variant as result,

typedef struct callable callable;

typedef execution_outcome callable_handler(
    list *positional_args,
    dict *named_args,
    void *callable_data, // used for AST nodes
    variant *this_obj,
    exec_context *ctx
);


callable *new_callable(const char *name, callable_handler *func, void *callable_data);

const char *callable_name(callable *c);
execution_outcome callable_call(callable *c, list *positional_args, dict *named_args, variant *this_obj, exec_context *ctx);
bool callables_are_equal(callable *a, callable *b);
const void callable_describe(callable *c, str_builder *sb);

extern class *callable_class;

#endif
