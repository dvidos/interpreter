#ifndef _VARIANT_H
#define _VARIANT_H

#include <stdbool.h>
#include "../../containers/_module.h"

#include "int_variant.h"
#include "str_variant.h"
#include "bool_variant.h"

typedef struct callable callable;
typedef struct variant variant;

variant *new_null_variant();
variant *new_bool_variant(bool b);
variant *new_int_variant(int i);
variant *new_float_variant(float f);
variant *new_str_variant(const char *p);
variant *new_list_variant(list *l);
variant *new_dict_variant(dict *d);
variant *new_callable_variant(callable *c);
variant *new_exception_variant(const char *script_filename, int script_line, int script_column, variant *inner, const char *fmt, ...);

bool variant_is_null(variant *v);
bool variant_is_bool(variant *v);
bool variant_is_int(variant *v);
bool variant_is_float(variant *v);
bool variant_is_str(variant *v);
bool variant_is_list(variant *v);
bool variant_is_dict(variant *v);
bool variant_is_callable(variant *v);
bool variant_is_exception(variant *v);

bool variant_as_bool(variant *v);
int variant_as_int(variant *v);
float variant_as_float(variant *v);
const char *variant_as_str(variant *v);
list *variant_as_list(variant *v);
dict *variant_as_dict(variant *v);
callable *variant_as_callable(variant *v);

bool variants_are_equal(variant *a, variant *b);
const void variant_describe(variant *v, str_builder *sb);


extern class *variant_class;



#endif
