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
    }
};

expression *new_expression(expr_type type);
expression *new_unary_expression(expr_type type, expression *operand);
expression *new_binary_expression(expr_type type, expression *left, expression *right);
expression *new_ternary_expression(expr_type type, expression *op1, expression *op2, expression *op3);

value *execute_expression(expr_node *expr, dict *values) {

}

