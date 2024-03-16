#ifndef _DICT_VARIANT_H
#define _DICT_VARIANT_H

#include "variant_type.h"

extern variant_type *dict_type;

variant *new_dict_variant();
variant *new_dict_variant_of(int entries_count, ...);
variant *new_dict_variant_owning(dict *dict);

dict *dict_variant_as_dict(variant *v); // caller should not free result
void dict_variant_set(variant *v, variant *key, variant *item);

#endif
