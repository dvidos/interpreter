#ifndef _FLOAT_VARIANT_H
#define _FLOAT_VARIANT_H

#include "../framework/_module.h"

extern variant_type *float_type;

variant *new_float_variant(float value);
float float_variant_as_float(variant *v);

#endif
