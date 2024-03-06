#ifndef _EXCEPTION_VARIANT_H
#define _EXCEPTION_VARIANT_H

#include "variant_type.h"

extern variant_type *exception_type;

variant *new_exception_variant(const char *filename, int line, int column, variant *inner, const char *fmt, ...);

#endif
