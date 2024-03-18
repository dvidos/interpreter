#ifndef _CALLABLE_VARIANT_H
#define _CALLABLE_VARIANT_H

#include "variant_type.h"

extern variant_type *callable_type;

typedef execution_outcome (*callable_variant_call_handler)(

    // passed along at call time
    list *positional_args,  // list of variants
    exec_context *ctx,      // execution_context / interpreter_state etc.

    // captured at callable creation time.
    void *payload,          // used for AST nodes etc (statements, expressions)
    variant *this,          // optional, in case of a method call (captured or direct)
    dict *captured_values   // optional, captured values in case of lambdas
    
);

typedef struct callable callable; // to avoid cyclical header inclusion

variant *new_callable_variant(callable *callable);
callable *callable_variant_as_callable(variant *v);



#endif
