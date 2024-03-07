#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "../utils/failable.h"
#include "statement_type.h"
#include "expression.h"


typedef struct statement statement;
extern item_info *statement_item_info;
struct statement {
    item_info *class;
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
        struct try_catch {
            list *try_statements;
            const char *exception_identifier;
            list *catch_statements;
            list *finally_statements;
        } try_catch;
        struct throw {
            expression *exception;
        } throw;
    } per_type;
    token *token;
};


statement *new_expression_statement(expression *expr);
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements, token *token);
statement *new_while_statement(expression *condition, list *body_statements, token *token);
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements, token *token);
statement *new_break_statement(token *token);
statement *new_continue_statement(token *token);
statement *new_return_statement(expression *value, token *token);
statement *new_function_statement(const char *name, list *arg_names, list *statements, token *token);
statement *new_try_catch_statement(list *try_statements, const char *exception_identifier, list *catch_statements, list *finally_statements, token *token);
statement *new_throw_statement(expression *exception, token *token);
statement *new_breakpoint_statement(token *token);

bool statement_is_at(statement *s, const char *filename, int line_no);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(statement);
#define failed_statement(inner, fmt, ...)  __failed_statement(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

bool statements_are_equal(statement *a, statement *b);
void statement_describe(statement *s, str_builder *sb);

#endif
