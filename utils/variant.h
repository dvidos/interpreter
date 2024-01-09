#ifndef _VALUE_H
#define _VALUE_H

#include <stdbool.h>
#include "failable.h"
#include "containers/list.h"
#include "containers/dict.h"

typedef struct variant variant;

variant *new_variant();
variant *new_bool_variant(bool b);
variant *new_int_variant(int i);
variant *new_float_variant(float f);
variant *new_str_variant(const char *p);
variant *new_list_variant(list *l);
variant *new_dict_variant(dict *d);

bool variant_is_null(variant *v);
bool variant_is_bool(variant *v);
bool variant_is_int(variant *v);
bool value_is_float(variant *v);
bool value_is_str(variant *v);
bool variant_is_list(variant *v);
bool variant_is_dict(variant *v);

bool variant_as_bool(variant *v);
int variant_as_int(variant *v);
float variant_as_float(variant *v);
const char *variant_as_str(variant *v);
list *variant_as_list(variant *v);
dict *variant_as_dict(variant *v);

bool variants_are_same(variant *a, variant *b);
const char *variant_to_string(variant *v);

STRONGLY_TYPED_FAILABLE_DECLARATION(variant);

#endif
