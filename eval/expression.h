#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/failable.h"
#include "../utils/value.h"
#include "../utils/dict.h"
#include "operator.h"


typedef enum expression_type {
    ET_IDENTIFIER,
    ET_NUMERIC_LITERAL,
    ET_STRING_LITERAL,
    ET_BOOLEAN_LITERAL,
    ET_UNARY_OP,
    ET_BINARY_OP,
    ET_TERNARY_OP,
    ET_FUNC_ARGS
} expression_type;

typedef struct expression expression;

expression *new_identifier_expression(const char *data);
expression *new_numeric_literal_expression(const char *data);
expression *new_string_literal_expression(const char *data);
expression *new_boolean_literal_expression(const char *data);

expression *new_unary_op_expression(operator op, expression *operand);
expression *new_binary_op_expression(operator op, expression *left, expression *right);
expression *new_ternary_op_expression(operator op, expression *op0, expression *op1, expression *op2);
expression *new_func_args_expression(list *args);

expression_type expression_get_type(expression *e);
operator expression_get_operator(expression *e);
int expression_get_operands_count(expression *e);
const char *expression_get_terminal_data(expression *e);
expression *expression_get_operand(expression *e, int index_1based);
list *expression_get_func_args(expression *e);

const char *expression_to_string(expression *e);
bool expressions_are_equal(expression *a, expression *b);

STRONGLY_TYPED_FAILABLE_DECLARATION(expression);


#endif
