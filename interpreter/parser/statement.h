#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "../../utils/failable.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/contained_item.h"
#include "expression.h"

typedef enum statement_type {
    ST_EXPRESSION,
    ST_IF,
    ST_WHILE,
    ST_CONTINUE,
    ST_BREAK,
    ST_FOR_LOOP,
} statement_type;

typedef struct statement statement;

statement *new_expression_statement(expression *expr);
statement *new_if_statement(expression *condition, list *body_statements);
statement *new_if_else_statement(expression *condition, list *body_statements, list *else_body_statements);
statement *new_while_statement(expression *condition, list *body_statements);
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements);
statement *new_break_statement();
statement *new_continue_statement();

extern contained_item *containing_statements;

STRONGLY_TYPED_FAILABLE_DECLARATION(statement);

#endif
