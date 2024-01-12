#ifndef _CALLABLE_H
#define _CALLABLE_H

#include "variant.h"
#include "containers/list.h"

// We want to support both callables being called,
// with a list of variants as arguments
// returning a variant as result,

typedef struct callable callable;


typedef failable_variant (callable_func)(list *arguments);

callable *new_callable(const char *name, callable_func *func, list *arg_types, variant_type ret_type);
failable_variant callable_call(callable *, list *arguments)


#endif
