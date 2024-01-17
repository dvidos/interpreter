#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "../../utils/failable.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/contained_item.h"
#include "../parser_expr/expression.h"

typedef enum statement_type {
    ST_EXPRESSION,
    ST_IF,
    ST_WHILE,
    ST_FOR_LOOP,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
} statement_type;

typedef struct statement statement;

statement *new_expression_statement(expression *expr);
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements);
statement *new_while_statement(expression *condition, list *body_statements);
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements);
statement *new_break_statement();
statement *new_continue_statement();
statement *new_return_statement(expression *value);

extern contained_item *containing_statements;

statement_type statement_get_type(statement *s);
expression *statement_get_expression(statement *s, int mnemonic);
bool statement_has_alternate_body(statement *s);
list *statement_get_statements_body(statement *s, bool alternative);


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(statement);

bool statements_are_equal(statement *a, statement *b);
const char *statement_to_string(statement *s);

#endif
