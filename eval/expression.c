#include "expression.h"

struct expression {
    enum expr_type type;
    char *name; // for symbols or var names
    union {
        struct {
            struct expr_node *operand;
        } unary;
        struct {
            struct expr_node *left;
            struct expr_node *right;
        } binary;
        struct {
            struct expr_node *op1;
            struct expr_node *op2;
            struct expr_node *op3;
        } ternary;
    } per_type;
};

expression *new_expression(expr_type type);
expression *new_unary_expression(expr_type type, expression *operand);
expression *new_binary_expression(expr_type type, expression *left, expression *right);
expression *new_ternary_expression(expr_type type, expression *op1, expression *op2, expression *op3);

value *execute_expression(expression *expr, dict *values) {
    // depending on whether it's a unary, binary, ternary expression,
    // evaluate deeper nodes first, then evaluate self.
    // have to apply conversions as needed (e.g. int to string etc)
    return new_null_value();
}

