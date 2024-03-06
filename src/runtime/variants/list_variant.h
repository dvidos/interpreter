#ifndef _LIST_VARIANT_H
#define _LIST_VARIANT_H

#include "variant_type.h"

extern variant_type *list_type;

variant *new_list_variant();
list *list_variant_as_list(variant *v); // caller should not free result

#endif