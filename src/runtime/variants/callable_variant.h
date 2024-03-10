#ifndef _CALLABLE_VARIANT_H
#define _CALLABLE_VARIANT_H

#include "variant_type.h"

extern variant_type *callable_type;

typedef struct callable callable; // to avoid cyclical header inclusion

variant *new_callable_variant(callable *callable, variant *member_parent);
callable *callable_variant_as_callable(variant *v); // caller should not free result


#endif
