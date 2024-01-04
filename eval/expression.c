#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/failable.h"
#include "../utils/containable.h"
#include "../utils/strbuff.h"
#include "expression.h"

typedef enum expression_type {
    ET_IDENTIFIER,
    ET_NUMERIC_LITERAL,
    ET_STRING_LITERAL,
    ET_BOOLEAN_LITERAL,
    
    ET_UNARRY_OP,
    ET_BINARY_OP,
    ET_TERNARY_OP,
    ET_FUNC_ARGS
} expression_type;

struct expression {
    containable *containable;
    expression_type type;
    operator op;
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

static expression *new_expression(expression_type type, operator op) {
    expression *e = malloc(sizeof(expression));
    memset(e, 0, sizeof(expression));
    e->containable = new_containable("expression",
        (are_equal_func)expressions_are_equal,
        (to_string_func)expression_to_string
    );
    e->type = type;
    e->op = op;
    return e;
}

expression *new_identifier_expression(const char *data) {
    expression *e = new_expression(ET_IDENTIFIER, OP_UNKNOWN);
    e->per_type.terminal.data = data;
    return e;
}

expression *new_numeric_literal_expression(const char *data) {
    expression *e = new_expression(ET_NUMERIC_LITERAL, OP_UNKNOWN);
    e->per_type.terminal.data = data;
    return e;
}

expression *new_string_literal_expression(const char *data) {
    expression *e = new_expression(ET_STRING_LITERAL, OP_UNKNOWN);
    e->per_type.terminal.data = data;
    return e;
}

expression *new_boolean_literal_expression(const char *data) {
    expression *e = new_expression(ET_BOOLEAN_LITERAL, OP_UNKNOWN);
    e->per_type.terminal.data = data;
    return e;
}

expression *new_unary_op_expression(operator op, expression *operand) {
    expression *e = new_expression(ET_UNARRY_OP, op);
    e->per_type.unary.operand = operand;
    return e;
}

expression *new_binary_op_expression(operator op, expression *left, expression *right) {
    expression *e = new_expression(ET_BINARY_OP, op);
    e->per_type.binary.left = left;
    e->per_type.binary.right = right;
    return e;
}

expression *new_ternary_op_expression(operator op, expression *op1, expression *op2, expression *op3) {
    expression *e = new_expression(ET_TERNARY_OP, op);
    e->per_type.ternary.op1 = op1;
    e->per_type.ternary.op2 = op2;
    e->per_type.ternary.op3 = op3;
    return e;
}

expression *new_func_args_expression(list *args) {
    expression *e = new_expression(ET_FUNC_ARGS, OP_UNKNOWN);
    e->per_type.func_args.list = args;
    return e;
}

bool expressions_are_equal(expression *a, expression *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->op != b->op)
        return false;
    if (a->type != b->type)
        return false;
    if (a->type == ET_IDENTIFIER || a->type == ET_NUMERIC_LITERAL || a->type == ET_STRING_LITERAL || a->type == ET_BOOLEAN_LITERAL) {
        if (strcmp(a->per_type.terminal.data, b->per_type.terminal.data) != 0)
            return false;
    } else if (a->type == ET_UNARRY_OP) {
        if (!expressions_are_equal(a->per_type.unary.operand, b->per_type.unary.operand))
            return false;
    } else if (a->type == ET_BINARY_OP) {
        if (!expressions_are_equal(a->per_type.binary.left, b->per_type.binary.left))
            return false;
        if (!expressions_are_equal(a->per_type.binary.right, b->per_type.binary.right))
            return false;
    } else if (a->type == ET_TERNARY_OP) {
        if (!expressions_are_equal(a->per_type.ternary.op1, b->per_type.ternary.op1))
            return false;
        if (!expressions_are_equal(a->per_type.ternary.op2, b->per_type.ternary.op2))
            return false;
        if (!expressions_are_equal(a->per_type.ternary.op3, b->per_type.ternary.op3))
            return false;
    } else if (a->type == ET_FUNC_ARGS) {
        if (!lists_are_equal(a->per_type.func_args.list, b->per_type.func_args.list))
            return false;
    }

    return true;
}

const char *expression_to_string(expression *e) {
    strbuff *s = new_strbuff();
    
    if (e->type == ET_IDENTIFIER) {
        strbuff_catf(s, "IDENTIFIER(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_NUMERIC_LITERAL) {
        strbuff_catf(s, "NUMBER(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_STRING_LITERAL) {
        strbuff_catf(s, "STRING(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_BOOLEAN_LITERAL) {
        strbuff_catf(s, "BOOLEAN(%s)", e->per_type.terminal.data);
    } else if (e->type == ET_UNARRY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.unary.operand));
        strbuff_catc(s, ')');
    } else if (e->type == ET_BINARY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.binary.left));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.binary.right));
        strbuff_catc(s, ')');
    } else if (e->type == ET_TERNARY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op1));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op2));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.ternary.op3));
        strbuff_catc(s, ')');
    } else if (e->type == ET_FUNC_ARGS) {
        strbuff_catf(s, "[", operator_str(e->op));
        sequential *seq = list_sequential(e->per_type.func_args.list);
        int num = 0;
        while (seq != NULL) {
            if (num++ > 0)
                strbuff_cat(s, ", ");
            strbuff_cat(s, expression_to_string((expression *)seq->data));
            seq = seq->next;
        }
        strbuff_catc(s, ']');
    }

    return strbuff_charptr(s);
}

failable_value execute_expression(expression *expr, dict *values) {
    // depending on whether it's a unary, binary, ternary expression,
    // evaluate deeper nodes first, then evaluate self.
    // have to apply conversions as needed (e.g. int to string etc)

    return failed_value("execute_expression() is not implemented yet!");
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(expression);
