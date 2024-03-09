#ifndef _EXCEPTION_VARIANT_H
#define _EXCEPTION_VARIANT_H

#include "variant_type.h"

extern variant_type *exception_type;

// TODO: make this "new_exception_variant_at()" for traced, add extra function to enrich with trace info (file/line) later.
variant *new_exception_variant(const char *filename, int line, int column, variant *inner, const char *fmt, ...);

#endif
