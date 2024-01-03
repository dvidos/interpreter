#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/failable.h"
#include "../utils/value.h"
#include "../utils/dict.h"
#include "operator.h"

typedef struct expression expression;

expression *new_identifier_expression(const char *data);
expression *new_numeric_literal_expression(const char *data);
expression *new_string_literal_expression(const char *data);
expression *new_boolean_literal_expression(const char *data);

expression *new_unary_op_expression(operator op, expression *operand);
expression *new_binary_op_expression(operator op, expression *left, expression *right);
expression *new_ternary_op_expression(operator op, expression *op1, expression *op2, expression *op3);
expression *new_func_args_expression(list *args);




const char *expression_to_string(expression *e);
bool expressions_are_equal(expression *a, expression *b);

failable_value execute_expression(expression *expr, dict *values);


STRONGLY_TYPED_FAILABLE_DECLARATION(expression);


#endif
