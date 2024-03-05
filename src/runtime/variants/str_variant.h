#ifndef _STR_VARIANT_H
#define _STR_VARIANT_H

#include "variant_type.h"

extern variant_type *str_type;

variant *new_str_variant(const char *value);
const char *str_variant_as_str(variant *v); // caller does not need to free result

#endif
