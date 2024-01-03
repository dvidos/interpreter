#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/failable.h"
#include "../utils/value.h"
#include "../utils/dict.h"
#include "operator.h"

typedef struct expression expression;

expression *new_terminal_expression(operator op, const char *data);
expression *new_unary_expression(operator op, expression *operand);
expression *new_binary_expression(operator op, expression *left, expression *right);
expression *new_ternary_expression(operator op, expression *op1, expression *op2, expression *op3);
expression *new_func_args_expression(list *args);

void expression_print(expression *e, FILE *stream, char *prefix, bool single_line);
void expression_print_list(list *expressions, FILE *stream, char *prefix, bool single_lines);

const char*expression_to_string(expression *e);
bool expressions_are_equal(expression *a, expression *b);

failable_value execute_expression(expression *expr, dict *values);


STRONGLY_TYPED_FAILABLE_DECLARATION(expression);


#endif
