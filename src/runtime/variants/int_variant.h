#ifndef _INT_VARIANT_H
#define _INT_VARIANT_H

#include "variant_type.h"

extern variant_type *int_type;

variant *new_int_variant(int value);
int int_variant_as_int(variant *v);

#endif
