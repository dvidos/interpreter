#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/failable.h"
#include "../utils/data_types/_module.h"
#include "../containers/_module.h"
#include "token.h"
#include "operator_type.h"
#include "expression_type.h"

typedef struct expression expression;
extern item_info *expression_item_info;

struct expression {
    item_info *class;
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
            const char *name;
            list *arg_names;
            list *statements;
        } func;
    } per_type;
};



expression *new_identifier_expression(const char *data, token *token);
expression *new_numeric_literal_expression(const char *data, token *token);
expression *new_string_literal_expression(const char *data, token *token);
expression *new_boolean_literal_expression(const char *data, token *token);
expression *new_unary_expression(operator_type op, token *token, expression *operand);
expression *new_binary_expression(operator_type op, token *token, expression *left, expression *right);
expression *new_list_data_expression(list *data, token *token);
expression *new_dict_data_expression(dict *data, token *token);
expression *new_func_decl_expression(const char *name, list *arg_names, list *statements, token *token);

const void expression_describe(expression *e, str_builder *sb);
bool expressions_are_equal(expression *a, expression *b);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(expression);
#define failed_expression(inner, fmt, ...)  __failed_expression(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
