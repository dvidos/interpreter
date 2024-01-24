#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "variant.h"
#include "../containers/_module.h"

// We want to support both callables being called,
// with a list of variants as arguments
// returning a variant as result,

typedef struct callable callable;


typedef failable_variant (callable_func)(list *arguments);

callable *new_callable(const char *name, const char *description, callable_func *func, variant_type ret_type, list *arg_types, bool var_args);

const char *callable_name(callable *c);
const char *callable_description(callable *c);
failable_variant callable_call(callable *c, list *arguments);
bool callables_are_equal(callable *a, callable *b);

extern contained_item *containing_callables;

#endif