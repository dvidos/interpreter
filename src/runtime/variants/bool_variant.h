#ifndef _BOOL_VARIANT_H
#define _BOOL_VARIANT_H

#include "../framework/_module.h"

extern variant_type *bool_type;

variant *new_bool_variant(bool value);
bool bool_variant_as_bool(variant *v);

#endif
