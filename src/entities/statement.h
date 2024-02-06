#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "../utils/failable.h"
#include "statement_type.h"
#include "expression.h"


typedef struct statement statement;

statement *new_expression_statement(expression *expr);
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements);
statement *new_while_statement(expression *condition, list *body_statements);
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements);
statement *new_break_statement();
statement *new_continue_statement();
statement *new_return_statement(expression *value);
statement *new_function_statement(const char *name, list *arg_names, list *statements);

extern class *statement_class;

statement_type statement_get_type(statement *s);
expression *statement_get_expression(statement *s, int mnemonic);
bool statement_has_alternate_body(statement *s);
list *statement_get_statements_body(statement *s, bool alternative);
const char *statement_get_function_name(statement *s);
list *statement_get_function_arg_names(statement *s);


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(statement);
#define failed_statement(inner, fmt, ...)  __failed_statement(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

bool statements_are_equal(statement *a, statement *b);
void statement_describe(statement *s, str_builder *sb);

#endif
