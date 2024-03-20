#ifndef _EXCEPTION_VARIANT_H
#define _EXCEPTION_VARIANT_H

#include "../framework/_module.h"

extern variant_type *exception_type;

variant *new_exception_variant(const char *fmt, ...);
variant *new_exception_variant_at(origin *origin, variant *inner, const char *fmt, ...);


#endif
