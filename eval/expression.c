#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/failable.h"
#include "../utils/containable.h"
#include "../utils/strbuff.h"
#include "expression.h"

struct expression {
    containable *containable;
    operator op;
    int operand_count;
    union {
        struct terminal {
            const char *data;
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
        struct func_args {
            list *list;
        } func_args;
    } per_type;
};

#define FUNC_ARGS_OPERAND_COUNT    999

expression *new_terminal_expression(operator op, const char *data) {
    expression *e = malloc(sizeof(expression));
    e->containable = new_containable("expression",
        (are_equal_func)expressions_are_equal,
        (to_string_func)expression_to_string
    );
    e->op = op;
    e->operand_count = 0;
    e->per_type.terminal.data = data;
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

expression *new_func_args_expression(list *args) {
    expression *e = malloc(sizeof(expression));
    e->op = OP_FUNC_ARGS;
    e->operand_count = FUNC_ARGS_OPERAND_COUNT;
    e->per_type.func_args.list = args;
    return e;
}

bool expressions_are_equal(expression *a, expression *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->op != b->op)
        return false;
    if (a->operand_count != b->operand_count)
        return false;
    if (a->operand_count == FUNC_ARGS_OPERAND_COUNT) {
        if (!lists_are_equal(a->per_type.func_args.list, b->per_type.func_args.list))
            return false;
    } else if (a->operand_count == 0) {
        if (strcmp(a->per_type.terminal.data, b->per_type.terminal.data) != 0)
            return false;
    } else if (a->operand_count == 1) {
        if (!expressions_are_equal(a->per_type.unary.operand, b->per_type.unary.operand))
            return false;
    } else if (a->operand_count == 2) {
        if (!expressions_are_equal(a->per_type.binary.left, b->per_type.binary.left))
            return false;
        if (!expressions_are_equal(a->per_type.binary.right, b->per_type.binary.right))
            return false;
    } else if (a->operand_count == 3) {
        if (!expressions_are_equal(a->per_type.ternary.op1, b->per_type.ternary.op1))
            return false;
        if (!expressions_are_equal(a->per_type.ternary.op2, b->per_type.ternary.op2))
            return false;
        if (!expressions_are_equal(a->per_type.ternary.op3, b->per_type.ternary.op3))
            return false;
    }

    return true;
}

const char *expression_to_string(expression *e) {
    strbuff *s = new_strbuff();
    
    strbuff_catf(s, "%s(", operator_str(e->op));
    if (e->operand_count == FUNC_ARGS_OPERAND_COUNT) {
        sequential *seq = list_sequential(e->per_type.func_args.list);
        int num = 0;
        while (seq != NULL) {
            if (num++ > 0)
                strbuff_cat(s, ", ");
            strbuff_cat(s, expression_to_string((expression *)seq->data));
            seq = seq->next;
        }
    } else if (e->operand_count == 0) {
        strbuff_catf(s, "\"%s\"", e->per_type.terminal.data);
    } else if (e->operand_count == 1) {
        strbuff_cat(s, expression_to_string(e->per_type.unary.operand));
    } else if (e->operand_count == 2) {
        strbuff_cat(s, expression_to_string(e->per_type.binary.left));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.binary.right));
    } else if (e->operand_count == 3) {
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op1));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op2));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op3));
    }
    strbuff_catc(s, ')');

    return strbuff_charptr(s);
}

failable_value execute_expression(expression *expr, dict *values) {
    // depending on whether it's a unary, binary, ternary expression,
    // evaluate deeper nodes first, then evaluate self.
    // have to apply conversions as needed (e.g. int to string etc)

    return failed_value("execute_expression() is not implemented yet!");
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(expression);
