#ifndef _VARIANT_H
#define _VARIANT_H

#include <stdbool.h>
#include "../../containers/_module.h"

typedef struct callable callable;
typedef struct variant variant;

// variant *new_null_variant();
// variant *new_bool_variant(bool b);
// variant *new_int_variant(int i);
// variant *new_float_variant(float f);
// variant *new_str_variant(const char *fmt, ...);
// variant *new_list_variant(list *l);
// variant *new_dict_variant(dict *d);
// variant *new_callable_variant(callable *c);
// variant *new_exception_variant(const char *script_filename, int script_line, int script_column, variant *inner, const char *fmt, ...);


const char *variant_as_str(variant *v);

bool variants_are_equal(variant *a, variant *b);
const void variant_describe(variant *v, str_builder *sb);


extern item_info *variant_item_info;



#endif
