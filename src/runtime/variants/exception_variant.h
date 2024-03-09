#ifndef _EXCEPTION_VARIANT_H
#define _EXCEPTION_VARIANT_H

#include "variant_type.h"

extern variant_type *exception_type;

variant *new_exception_variant(const char *fmt, ...);
variant *new_exception_variant_at(const char *filename, int line, int column, variant *inner, const char *fmt, ...);
void exception_variant_set_source(variant *v, const char *filename, int line, int column);


#endif
