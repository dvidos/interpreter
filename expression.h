#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "value.h"
#include "dict.h"


typedef enum expr_type {
    VAR_NAME,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    BINARY_NOT, // unary
    BINARY_AND,
    BINARY_OR,
    SHORTHAND_IF // ternary, where 1=cond, 2=true, 3=false
} expr_type;

typedef struct expression expression;


expression *new_expression(expr_type type);
expression *new_unary_expression(expr_type type, expression *operand);
expression *new_binary_expression(expr_type type, expression *left, expression *right);
expression *new_ternary_expression(expr_type type, expression *op1, expression *op2, expression *op3);

value *execute_expression(expression *expr, dict *values);



#endif
