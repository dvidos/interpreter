#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/value.h"
#include "../utils/dict.h"
#include "operator.h"

typedef struct expression expression;

expression *new_terminal_expression(operator op);
expression *new_unary_expression(operator op, expression *operand);
expression *new_binary_expression(operator op, expression *left, expression *right);
expression *new_ternary_expression(operator op, expression *op1, expression *op2, expression *op3);

value *execute_expression(expression *expr, dict *values);



#endif
