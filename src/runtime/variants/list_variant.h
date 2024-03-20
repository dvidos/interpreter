#ifndef _LIST_VARIANT_H
#define _LIST_VARIANT_H

#include "../framework/_framework.h"

extern variant_type *list_type;

variant *new_list_variant();
variant *new_list_variant_of(int argc, ...);
variant *new_list_variant_owning(list *list);

list *list_variant_as_list(variant *v); // caller should not free result

#endif
