#ifndef _VALUE_H
#define _VALUE_H

#include <stdbool.h>
#include "failable.h"

typedef struct value value;

value *new_null_value();
value *new_bool_value(bool b);
value *new_int_value(int i);
value *new_float_value(float f);
value *new_str_value(char *p);

bool value_is_null(value *v);
bool value_is_bool(value *v);
bool value_is_int(value *v);
bool value_is_float(value *v);
bool value_is_str(value *v);

bool value_as_bool(value *v);
int value_as_int(value *v);
float value_as_float(value *v);
const char *value_as_str(value *v);


STRONGLY_TYPED_FAILABLE_DECLARATION(value);

#endif
