#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/failable.h"
#include "expression.h"

struct expression {
    operator op;
    int operand_count; // 0=terminal, 1=unary, 2=binary, 3=ternary
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

expression *new_terminal_expression(operator op, const char *data) {
    expression *e = malloc(sizeof(expression));
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
    e->per_type.func_args.list = args;
    return e;
}

static void expression_print_indented(expression *e, FILE *stream, char *prefix, bool single_line, int level) {
    if (level == 0)
        fprintf(stream, "%s", prefix);
    
    fprintf(stream, "%s(", operator_str(e->op));
    if (e->op == OP_FUNC_ARGS) {
        sequential *s = list_sequential(e->per_type.func_args.list);
        int num = 0;
        while (s != NULL) {
            if (num++ > 0)
                fprintf(stream, ", ");
            expression_print_indented((expression *)s->data, stream, "", single_line, level + 1);
            s = s->next;
        }
    } else if (e->operand_count == 0) {
        fprintf(stream, "\"%s\"", e->per_type.terminal.data);
    } else if (e->operand_count == 1) {
        expression_print_indented(e->per_type.unary.operand, stream, "", single_line, level + 1);
    } else if (e->operand_count == 2) {
        expression_print_indented(e->per_type.binary.left, stream, "", single_line, level + 1);
        fprintf(stream, ", ");
        expression_print_indented(e->per_type.binary.right, stream, "", single_line, level + 1);
    } else if (e->operand_count == 3) {
        expression_print_indented(e->per_type.ternary.op1, stream, "", single_line, level + 1);
        fprintf(stream, ", ");
        expression_print_indented(e->per_type.ternary.op2, stream, "", single_line, level + 1);
        fprintf(stream, ", ");
        expression_print_indented(e->per_type.ternary.op3, stream, "", single_line, level + 1);
    }
    fprintf(stream, ")");

    if (level == 0)
        fprintf(stream, "\n");
    
}

void expression_print(expression *e, FILE *stream, char *prefix, bool single_line) {
    expression_print_indented(e, stream, prefix, single_line, 0);
}

void expression_print_list(list *expressions, FILE *stream, char *prefix, bool single_lines) {
    for (sequential *s = list_sequential(expressions); s != NULL; s = s->next)
        expression_print((expression *)s->data, stream, prefix, single_lines);
}

bool expressions_are_equal(expression *a, expression *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->op != b->op)
        return false;
    if (a->operand_count != b->operand_count)
        return false;
    if (a->op == OP_FUNC_ARGS) {
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


failable_value execute_expression(expression *expr, dict *values) {
    // depending on whether it's a unary, binary, ternary expression,
    // evaluate deeper nodes first, then evaluate self.
    // have to apply conversions as needed (e.g. int to string etc)

    return failed_value("execute_expression() is not implemented yet!");
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(expression);
