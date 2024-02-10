#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "../utils/failable.h"
#include "statement_type.h"
#include "expression.h"


typedef struct statement statement;
extern class *statement_class;
struct statement {
    class *class;
    statement_type type;
    union {
        struct expr {
            expression *expr;
        } expr;
        struct if_ {
            expression *condition;
            list *body_statements;
            bool has_else;
            list *else_body_statements;
        } if_;
        struct while_ {
            expression *condition;
            list *body_statements;
        } while_;
        struct for_ {
            expression *init;
            expression *condition;
            expression *next;
            list *body_statements;
        } for_;
        struct return_ {
            expression *value;
        } return_;
        struct function {
            const char *name;
            list *arg_names;
            list *statements;
        } function;
    } per_type;
};


statement *new_expression_statement(expression *expr);
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements);
statement *new_while_statement(expression *condition, list *body_statements);
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements);
statement *new_break_statement();
statement *new_continue_statement();
statement *new_return_statement(expression *value);
statement *new_function_statement(const char *name, list *arg_names, list *statements);
statement *new_breakpoint_statement();


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(statement);
#define failed_statement(inner, fmt, ...)  __failed_statement(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

bool statements_are_equal(statement *a, statement *b);
void statement_describe(statement *s, str_builder *sb);

#endif
