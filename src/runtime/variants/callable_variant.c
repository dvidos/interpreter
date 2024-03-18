#include "_internal.h"
#include "../../utils/hash.h"
#include "../../utils/data_types/callable.h"
#include <string.h>
#include <stdio.h>


// the callable class must be callable by design,
// it will be used for base functions, class methods (bound to 'this'),
// even anonymous functions (bound to variables at the time of evaluation)

typedef struct callable_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;

    const char *name;       // for debugging, stack trace etc.
    void *payload;          // used for AST nodes etc (statements, expressions)
    variant *this;          // optional, in case of a method call (captured or direct)
    dict *captured_values;  // optional, captured values in case of lambdas

    callable *callable;
} callable_instance;

static variant *instance_stringify(callable_instance *obj) {
    if (obj->callable == NULL)
        return NULL;
    
    if (obj->this != NULL)
        return new_str_variant("%s.%s()", obj->this->_type->name, obj->name);
    else if (obj->name != NULL)
        return new_str_variant("%s()", obj->name);
    else
        return new_str_variant("(callable @ 0x%p", obj);
}

static execution_outcome instance_call(variant *obj, list *args, variant *this_obj, origin *call_origin, exec_context *ctx) {
    callable_instance *c = (callable_instance *)obj;
    if (c->callable == NULL)
        return failed_outcome("Callable variant was not correctly setup");
    
    return callable_call(c->callable, args, NULL, call_origin, ctx);
}

variant_type *callable_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "callable",
    .parent_type = NULL,
    .instance_size = sizeof(callable_instance),

    .stringifier = (stringifier_func)instance_stringify,
    .call_handler = instance_call, // don't cast, so that we get warnings when handler arguments change.
};

variant *new_callable_variant(callable *c) {
    execution_outcome ex = variant_create(callable_type, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    callable_instance *obj = (callable_instance *)ex.result;
    obj->callable = c;
    return (variant *)obj;
}

callable *callable_variant_as_callable(variant *v) {
    if (!variant_instance_of(v, callable_type))
        return NULL;
    return ((callable_instance *)v)->callable;
}
