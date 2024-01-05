#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "../utils/value.h"
#include "../utils/dict.h"
#include "operator.h"


// failable_value expression_execute(expression *expr, dict *values);

failable_value execute_unary_operation(operator op, value *value, dict *values);
failable_value execute_binary_operation(operator op, value *v1, value *v2, dict *values);
failable_value execute_ternary_operation(operator op, value *v1, value *v2, value *v3, dict *values);


#endif
