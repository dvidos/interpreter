#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "../utils/failable.h"
#include "../utils/data_types/_module.h"
#include "../utils/containers/_module.h"
#include "token.h"
#include "operator_type.h"
#include "expression_type.h"

typedef struct expression expression;

extern contained_item *containing_expressions;


expression *new_identifier_expression(const char *data, token *token);
expression *new_numeric_literal_expression(const char *data, token *token);
expression *new_string_literal_expression(const char *data, token *token);
expression *new_boolean_literal_expression(const char *data, token *token);

expression *new_unary_op_expression(operator_type op, token *token, expression *operand);
expression *new_binary_op_expression(operator_type op, token *token, expression *left, expression *right);
expression *new_list_data_expression(list *data, token *token);
expression *new_dict_data_expression(dict *data, token *token);
expression *new_func_decl_expression(list *arg_names, list *statements, token *token);

expression_type expression_get_type(expression *e);
token *expression_get_token(expression *e);
operator_type expression_get_operator(expression *e);
int expression_get_operands_count(expression *e);
const char *expression_get_terminal_data(expression *e);
expression *expression_get_operand(expression *e, int index);
list *expression_get_list_data(expression *e);
dict *expression_get_dict_data(expression *e);
list *expression_get_func_statements(expression *e);
list *expression_get_func_arg_names(expression *e);

const char *expression_to_string(expression *e);
bool expressions_are_equal(expression *a, expression *b);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(expression);
#define failed_expression(inner, fmt, ...)  __failed_expression(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
