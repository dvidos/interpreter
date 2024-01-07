#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/failable.h"
#include "../utils/containable.h"
#include "../utils/strbuff.h"
#include "expression.h"
#include "execution.h"

struct expression {
    containable *containable;
    expression_type type;
    operator op;
    union {
        struct terminal {
            const char *data;
        } terminal;
        struct ternary {
            struct expression *op0;
            struct expression *op1;
            struct expression *op2;
        } operation;
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
    expression *e = new_expression(ET_UNARY_OP, op);
    e->per_type.operation.op0 = operand;
    return e;
}

expression *new_binary_op_expression(operator op, expression *left, expression *right) {
    expression *e = new_expression(ET_BINARY_OP, op);
    e->per_type.operation.op0 = left;
    e->per_type.operation.op1 = right;
    return e;
}

expression *new_ternary_op_expression(operator op, expression *op0, expression *op1, expression *op2) {
    expression *e = new_expression(ET_TERNARY_OP, op);
    e->per_type.operation.op0 = op0;
    e->per_type.operation.op1 = op1;
    e->per_type.operation.op2 = op2;
    return e;
}

expression *new_func_args_expression(list *args) {
    expression *e = new_expression(ET_FUNC_ARGS, OP_UNKNOWN);
    e->per_type.func_args.list = args;
    return e;
}

expression_type expression_get_type(expression *e) {
    return e->type;
}

operator expression_get_operator(expression *e) {
    return e->op;
}

int expression_get_operands_count(expression *e) {
    switch (e->type) {
        case ET_IDENTIFIER:      // fallthrough
        case ET_NUMERIC_LITERAL: // fallthrough
        case ET_STRING_LITERAL:  // fallthrough
        case ET_BOOLEAN_LITERAL: // fallthrough
        case ET_FUNC_ARGS:
            return 0;
        case ET_UNARY_OP:        return 1;
        case ET_BINARY_OP:       return 2;
        case ET_TERNARY_OP:      return 3;
        default: return 0;
    }
}

const char *expression_get_terminal_data(expression *e) {
    return e->per_type.terminal.data;
}

expression *expression_get_operand(expression *e, int index) {
    switch (index) {
        case 0: return e->per_type.operation.op0;
        case 1: return e->per_type.operation.op1;
        case 2: return e->per_type.operation.op2;
        default: return 0;
    }
}

list *expression_get_func_args(expression *e) {
    return e->per_type.func_args.list;
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
    } else if (a->type == ET_UNARY_OP) {
        if (!expressions_are_equal(a->per_type.operation.op0, b->per_type.operation.op0))
            return false;
    } else if (a->type == ET_BINARY_OP) {
        if (!expressions_are_equal(a->per_type.operation.op0, b->per_type.operation.op0))
            return false;
        if (!expressions_are_equal(a->per_type.operation.op1, b->per_type.operation.op1))
            return false;
    } else if (a->type == ET_TERNARY_OP) {
        if (!expressions_are_equal(a->per_type.operation.op0, b->per_type.operation.op0))
            return false;
        if (!expressions_are_equal(a->per_type.operation.op1, b->per_type.operation.op1))
            return false;
        if (!expressions_are_equal(a->per_type.operation.op2, b->per_type.operation.op2))
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
    } else if (e->type == ET_UNARY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.operation.op0));
        strbuff_catc(s, ')');
    } else if (e->type == ET_BINARY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.operation.op0));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.operation.op1));
        strbuff_catc(s, ')');
    } else if (e->type == ET_TERNARY_OP) {
        strbuff_catf(s, "%s(", operator_str(e->op));
        strbuff_cat(s, expression_to_string(e->per_type.operation.op0));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.operation.op1));
        strbuff_cat(s, ", ");
        strbuff_cat(s, expression_to_string(e->per_type.operation.op2));
        strbuff_catc(s, ')');
    } else if (e->type == ET_FUNC_ARGS) {
        strbuff_catf(s, "[", operator_str(e->op));
        int num = 0;
        for_list(e->per_type.func_args.list, it, expression, arg_exp) {
            if (num++ > 0) strbuff_cat(s, ", ");
            strbuff_cat(s, expression_to_string(arg_exp));
        }
        strbuff_catc(s, ']');
    }

    return strbuff_charptr(s);
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(expression);
