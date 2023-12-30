#include <stdlib.h>
#include "expression.h"

typedef struct expression {
    operator op;
    int operand_count; // 0=terminal, 1=unary, 2=binary, 3=ternary
    union {
        struct terminal {
            char *data;
        } terminal;
        struct unary {
            struct expression *operand;
        } unary;
        struct binary {
            struct expression *left;
            struct expression *right;
        } binary;
        struct ternary {
            struct expression *op1;
            struct expression *op2;
            struct expression *op3;
        } ternary;
    } per_type;
} expression;

expression *new_terminal_expression(operator op) {
    expression *e = malloc(sizeof(expression));
    e->op = op;
    e->operand_count = 0;
    return e;
}

expression *new_unary_expression(operator op, expression *operand) {
    expression *e = malloc(sizeof(expression));
    e->op = op;
    e->operand_count = 1;
    e->per_type.unary.operand = operand;
    return e;
}

expression *new_binary_expression(operator op, expression *left, expression *right) {
    expression *e = malloc(sizeof(expression));
    e->op = op;
    e->operand_count = 2;
    e->per_type.binary.left = left;
    e->per_type.binary.right = right;
    return e;
}

expression *new_ternary_expression(operator op, expression *op1, expression *op2, expression *op3) {
    expression *e = malloc(sizeof(expression));
    e->op = op;
    e->operand_count = 3;
    e->per_type.ternary.op1 = op1;
    e->per_type.ternary.op2 = op2;
    e->per_type.ternary.op3 = op3;
    return e;
}

value *execute_expression(expression *expr, dict *values) {
    // depending on whether it's a unary, binary, ternary expression,
    // evaluate deeper nodes first, then evaluate self.
    // have to apply conversions as needed (e.g. int to string etc)

    return new_null_value();
}
