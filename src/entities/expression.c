#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../utils/str_builder.h"
#include "expression.h"

struct expression {
    expression_type type;
    token *token;
    operator_type op;
    union {
        const char *terminal_data;
        list *list_;
        dict *dict_;
        struct operation {
            struct expression *operand1;
            struct expression *operand2;
        } operation;
        struct func {
            list *arg_names;
            list *statements;
        } func;
    } per_type;
};

contained_item *containing_expressions = &(contained_item){
    .type_name = "expression",
    .are_equal = (are_equal_func)expressions_are_equal,
    .to_string = (describe_func)expression_describe,
    .hash      = NULL
};

static expression *new_expression(expression_type type, token *token, operator_type op) {
    expression *e = malloc(sizeof(expression));
    memset(e, 0, sizeof(expression));
    e->type = type;
    e->token = token;
    e->op = op;
    return e;
}

expression *new_identifier_expression(const char *data, token *token) {
    expression *e = new_expression(ET_IDENTIFIER, token, OP_UNKNOWN);
    e->per_type.terminal_data = data;
    return e;
}

expression *new_numeric_literal_expression(const char *data, token *token) {
    expression *e = new_expression(ET_NUMERIC_LITERAL, token, OP_UNKNOWN);
    e->per_type.terminal_data = data;
    return e;
}

expression *new_string_literal_expression(const char *data, token *token) {
    expression *e = new_expression(ET_STRING_LITERAL, token, OP_UNKNOWN);
    e->per_type.terminal_data = data;
    return e;
}

expression *new_boolean_literal_expression(const char *data, token *token) {
    expression *e = new_expression(ET_BOOLEAN_LITERAL, token, OP_UNKNOWN);
    e->per_type.terminal_data = data;
    return e;
}

expression *new_unary_expression(operator_type op, token *token, expression *operand) {
    expression *e = new_expression(ET_UNARY_OP, token, op);
    e->per_type.operation.operand1 = operand;
    return e;
}

expression *new_binary_expression(operator_type op, token *token, expression *left, expression *right) {
    expression *e = new_expression(ET_BINARY_OP, token, op);
    e->per_type.operation.operand1 = left;
    e->per_type.operation.operand2 = right;
    return e;
}

expression *new_list_data_expression(list *l, token *token) {
    expression *e = new_expression(ET_LIST_DATA, token, OP_UNKNOWN);
    e->per_type.list_ = l;
    return e;
}

expression *new_dict_data_expression(dict *d, token *token) {
    expression *e = new_expression(ET_DICT_DATA, token, OP_UNKNOWN);
    e->per_type.dict_ = d;
    return e;
}

expression *new_func_decl_expression(list *arg_names, list *statements, token *token) {
    expression *e = new_expression(ET_FUNC_DECL, token, OP_UNKNOWN);
    e->per_type.func.arg_names = arg_names;
    e->per_type.func.statements = statements;
    return e;
}

expression_type expression_get_type(expression *e) {
    return e->type;
}

token *expression_get_token(expression *e) {
    return e->token;
}

operator_type expression_get_operator(expression *e) {
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
            return 0;
        case ET_UNARY_OP:        return 1;
        case ET_BINARY_OP:       return 2;
        default: return 0;
    }
}

const char *expression_get_terminal_data(expression *e) {
    return e->per_type.terminal_data;
}

expression *expression_get_operand(expression *e, int index) {
    switch (index) {
        case 0: return e->per_type.operation.operand1;
        case 1: return e->per_type.operation.operand2;
        default: return 0;
    }
}

list *expression_get_list_data(expression *e) {
    return e->type == ET_LIST_DATA ? e->per_type.list_ : NULL;
}

dict *expression_get_dict_data(expression *e) {
    return e->type == ET_DICT_DATA ? e->per_type.dict_ : NULL;
}

list *expression_get_func_statements(expression *e) {
    return e->type == ET_FUNC_DECL ? e->per_type.func.statements : NULL;
}

list *expression_get_func_arg_names(expression *e) {
    return e->type == ET_FUNC_DECL ? e->per_type.func.arg_names : NULL;
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
        if (strcmp(a->per_type.terminal_data, b->per_type.terminal_data) != 0)
            return false;
    } else if (a->type == ET_UNARY_OP) {
        if (!expressions_are_equal(a->per_type.operation.operand1, b->per_type.operation.operand1))
            return false;
    } else if (a->type == ET_BINARY_OP) {
        if (!expressions_are_equal(a->per_type.operation.operand1, b->per_type.operation.operand1))
            return false;
        if (!expressions_are_equal(a->per_type.operation.operand2, b->per_type.operation.operand2))
            return false;
    } else if (a->type == ET_LIST_DATA) {
        if (!lists_are_equal(a->per_type.list_, b->per_type.list_))
            return false;
    } else if (a->type == ET_DICT_DATA) {
        if (!dicts_are_equal(a->per_type.dict_, b->per_type.dict_))
            return false;
    }

    return true;
}

const void expression_describe(expression *e, str_builder *sb) {
    
    if (e->type == ET_IDENTIFIER) {
        str_builder_addf(sb, "SYM(\"%s\")", e->per_type.terminal_data);
    } else if (e->type == ET_NUMERIC_LITERAL) {
        str_builder_addf(sb, "NUM(\"%s\")", e->per_type.terminal_data);
    } else if (e->type == ET_STRING_LITERAL) {
        str_builder_addf(sb, "STR(\"%s\")", e->per_type.terminal_data);
    } else if (e->type == ET_BOOLEAN_LITERAL) {
        str_builder_addf(sb, "BOOL(%s)", e->per_type.terminal_data);
    } else if (e->type == ET_UNARY_OP) {
        operator_type_describe(e->op, sb);
        str_builder_addc(sb, '(');
        expression_describe(e->per_type.operation.operand1, sb);
        str_builder_addc(sb, ')');
    } else if (e->type == ET_BINARY_OP) {
        operator_type_describe(e->op, sb);
        str_builder_addc(sb, '(');
        expression_describe(e->per_type.operation.operand1, sb);
        str_builder_add(sb, ", ");
        expression_describe(e->per_type.operation.operand2, sb);
        str_builder_addc(sb, ')');
    } else if (e->type == ET_LIST_DATA) {
        str_builder_add(sb, "LIST(");
        list_describe(e->per_type.list_, ", ", sb);
        str_builder_addc(sb, ')');
    } else if (e->type == ET_DICT_DATA) {
        str_builder_add(sb, "DICT(");
        dict_describe(e->per_type.dict_, ": ", ", ", sb);
        str_builder_add(sb, ")");
    } else if (e->type == ET_FUNC_DECL) {
        str_builder_add(sb, "FUNC(");
        list_describe(e->per_type.func.arg_names, ", ", sb);
        str_builder_addf(sb, "){ %d statements }", list_length(e->per_type.func.statements));
    }
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(expression);
