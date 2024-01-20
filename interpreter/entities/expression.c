#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/failable.h"
#include "../../utils/containers/_module.h"
#include "../../utils/str_builder.h"
#include "expression.h"

struct expression {
    expression_type type;
    operator op;
    union {
        struct terminal {
            const char *data;
        } terminal;
        struct operation {
            struct expression *op0;
            struct expression *op1;
        } operation;
        struct list_data {
            list *data;
        } list_data;
        struct dict_data {
            dict *data;
        } dict_data;
        struct pair {
            struct expression *left;
            struct expression *right;
        } pair;
    } per_type;
};

contained_item *containing_expressions = &(contained_item){
    .type_name = "expression",
    .are_equal = (are_equal_func)expressions_are_equal,
    .to_string = (to_string_func)expression_to_string,
    .hash      = NULL
};

static expression *new_expression(expression_type type, operator op) {
    expression *e = malloc(sizeof(expression));
    memset(e, 0, sizeof(expression));
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

expression *new_list_data_expression(list *l) {
    expression *e = new_expression(ET_LIST_DATA, OP_UNKNOWN);
    e->per_type.list_data.data = l;
    return e;
}

expression *new_dict_data_expression(dict *d) {
    expression *e = new_expression(ET_DICT_DATA, OP_UNKNOWN);
    e->per_type.dict_data.data = d;
    return e;
}

expression *new_pair_expression(expression *left, expression *right) {
    expression *e = new_expression(ET_EXPR_PAIR, OP_UNKNOWN);
    e->per_type.pair.left = left;
    e->per_type.pair.right = right;
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
        case ET_LIST_DATA:
        case ET_DICT_DATA:
        case ET_EXPR_PAIR:
            return 0;
        case ET_UNARY_OP:        return 1;
        case ET_BINARY_OP:       return 2;
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
        default: return 0;
    }
}

list *expression_get_list_data(expression *e) {
    return e->type == ET_LIST_DATA ? e->per_type.list_data.data : NULL;
}

dict *expression_get_dict_data(expression *e) {
    return e->type == ET_DICT_DATA ? e->per_type.dict_data.data : NULL;
}

expression *expression_get_pair_item(expression *e, bool left) {
    return left ? e->per_type.pair.left : e->per_type.pair.right;
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
    } else if (a->type == ET_LIST_DATA) {
        if (!lists_are_equal(a->per_type.list_data.data, b->per_type.list_data.data))
            return false;
    } else if (a->type == ET_DICT_DATA) {
        if (!dicts_are_equal(a->per_type.dict_data.data, b->per_type.dict_data.data))
            return false;
    } else if (a->type == ET_EXPR_PAIR) {
        if (!expressions_are_equal(a->per_type.pair.left, b->per_type.pair.left))
            return false;
        if (!expressions_are_equal(a->per_type.pair.right, b->per_type.pair.right))
            return false;
    }

    return true;
}

const char *expression_to_string(expression *e) {
    str_builder *sb = new_str_builder();
    
    if (e->type == ET_IDENTIFIER) {
        str_builder_catf(sb, "IDENTIFIER(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_NUMERIC_LITERAL) {
        str_builder_catf(sb, "NUMBER(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_STRING_LITERAL) {
        str_builder_catf(sb, "STRING(\"%s\")", e->per_type.terminal.data);
    } else if (e->type == ET_BOOLEAN_LITERAL) {
        str_builder_catf(sb, "BOOLEAN(%s)", e->per_type.terminal.data);
    } else if (e->type == ET_UNARY_OP) {
        str_builder_catf(sb, "%s(", operator_str(e->op));
        str_builder_cat(sb, expression_to_string(e->per_type.operation.op0));
        str_builder_catc(sb, ')');
    } else if (e->type == ET_BINARY_OP) {
        str_builder_catf(sb, "%s(", operator_str(e->op));
        str_builder_cat(sb, expression_to_string(e->per_type.operation.op0));
        str_builder_cat(sb, ", ");
        str_builder_cat(sb, expression_to_string(e->per_type.operation.op1));
        str_builder_catc(sb, ')');
    } else if (e->type == ET_LIST_DATA) {
        str_builder_cat(sb, "LIST(");
        str_builder_cat(sb, list_to_string(e->per_type.list_data.data, ", "));
        str_builder_catc(sb, ')');
    } else if (e->type == ET_DICT_DATA) {
        str_builder_cat(sb, "DICT(");
        str_builder_cat(sb, dict_to_string(e->per_type.dict_data.data, ":", ", "));
        str_builder_cat(sb, ")");
    } else if (e->type == ET_EXPR_PAIR) {
        str_builder_cat(sb, "PAIR(");
        str_builder_cat(sb, expression_to_string(e->per_type.pair.left));
        str_builder_cat(sb, ", ");
        str_builder_cat(sb, expression_to_string(e->per_type.pair.right));
        str_builder_catc(sb, ')');
    }

    return str_builder_charptr(sb);
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(expression);
