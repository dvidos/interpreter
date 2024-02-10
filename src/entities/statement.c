#include <stdlib.h>
#include "../utils/str.h"
#include "../utils/str_builder.h"
#include "../utils/containers/_module.h"
#include "statement.h"


static statement *new_statement(statement_type type, token *token) {
    statement *s = malloc(sizeof(statement));
    s->class = expression_class;
    s->type = type;
    s->token = token;
    return s;
}

statement *new_expression_statement(expression *expr) {
    statement *s = new_statement(ST_EXPRESSION, expr->token);
    s->per_type.expr.expr = expr;
    return s;
}
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements, token *token) {
    statement *s = new_statement(ST_IF, token);
    s->per_type.if_.condition = condition;
    s->per_type.if_.body_statements = body_statements;
    s->per_type.if_.has_else = has_else;
    s->per_type.if_.else_body_statements = else_body_statements;
    return s;
}
statement *new_while_statement(expression *condition, list *body_statements, token *token) {
    statement *s = new_statement(ST_WHILE, token);
    s->per_type.while_.condition = condition;
    s->per_type.while_.body_statements = body_statements;
    return s;
}
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements, token *token) {
    statement *s = new_statement(ST_FOR_LOOP, token);
    s->per_type.for_.init = init;
    s->per_type.for_.condition = condition;
    s->per_type.for_.next = next;
    s->per_type.for_.body_statements = body_statements;
    return s;
}
statement *new_break_statement(token *token) {
    return new_statement(ST_BREAK, token);
}
statement *new_continue_statement(token *token) {
    return new_statement(ST_CONTINUE, token);
}
statement *new_return_statement(expression *value, token *token) {
    statement *s = new_statement(ST_RETURN, token);
    s->per_type.return_.value = value;
    return s;
}
statement *new_function_statement(const char *name, list *arg_names, list *statements, token *token) {
    statement *s = new_statement(ST_FUNCTION, token);
    s->per_type.function.name = name;
    s->per_type.function.arg_names = arg_names;
    s->per_type.function.statements = statements;
    return s;
}
statement *new_breakpoint_statement(token *token) {
    return new_statement(ST_BREAKPOINT, token);
}



const void statement_describe(statement *s, str_builder *sb) {
    switch (s->type) {
        case ST_EXPRESSION:
            str_builder_add(sb, "expr(");
            expression_describe(s->per_type.expr.expr, sb);
            str_builder_add(sb, ")");
            break;
        case ST_IF:
            str_builder_add(sb, "if (");
            expression_describe(s->per_type.if_.condition, sb);
            str_builder_add(sb, ") {\n    ");
            list_describe(s->per_type.if_.body_statements, "\n    ", sb);
            if (s->per_type.if_.has_else) {
                str_builder_add(sb, "\n} else {\n    ");
                list_describe(s->per_type.if_.else_body_statements, "\n    ", sb);
            }
            str_builder_add(sb, "\n}");
            break;
        case ST_WHILE:
            str_builder_add(sb, "while (");
            expression_describe(s->per_type.while_.condition, sb);
            str_builder_add(sb, ") {\n    ");
            list_describe(s->per_type.while_.body_statements, "\n    ", sb);
            str_builder_add(sb, "\n}");
            break;
        case ST_FOR_LOOP:
            str_builder_add(sb, "for (");
            expression_describe(s->per_type.for_.init, sb);
            str_builder_add(sb, "; ");
            expression_describe(s->per_type.for_.condition, sb);
            str_builder_add(sb, "; ");
            expression_describe(s->per_type.for_.next, sb);
            str_builder_add(sb, ") {\n    ");
            list_describe(s->per_type.for_.body_statements, "\n    ", sb);
            str_builder_add(sb, "\n}");
            break;
        case ST_CONTINUE:
            str_builder_add(sb, "continue;");
            break;
        case ST_BREAK:
            str_builder_add(sb, "break;");
            break;
        case ST_RETURN:
            str_builder_add(sb, "return (");
            expression_describe(s->per_type.return_.value, sb);
            str_builder_add(sb, ");");
            break;
        case ST_FUNCTION:
            str_builder_add(sb, "function ");
            str_builder_add(sb, s->per_type.function.name == NULL ?
                    "(anonymous)": s->per_type.function.name);
            str_builder_add(sb, "(");
            list_describe(s->per_type.function.arg_names, ", ", sb);
            str_builder_add(sb, ") {\n    ");
            list_describe(s->per_type.function.statements, "\n    ", sb);
            str_builder_add(sb, "\n}");
            break;
    }
}

bool statements_are_equal(statement *a, statement *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    if (a == b) return true;
    if (a->type != b->type) return false;

    switch (a->type) {
        case ST_EXPRESSION:
            if (!expressions_are_equal(a->per_type.expr.expr, b->per_type.expr.expr)) return false;
            break;
        case ST_IF:
            if (!expressions_are_equal(a->per_type.if_.condition, b->per_type.if_.condition)) return false;
            if (!lists_are_equal(a->per_type.if_.body_statements, b->per_type.if_.body_statements)) return false;
            if (a->per_type.if_.has_else != b->per_type.if_.has_else) return false;
            if (!lists_are_equal(a->per_type.if_.else_body_statements, b->per_type.if_.else_body_statements)) return false;
            break;
        case ST_WHILE:
            if (!expressions_are_equal(a->per_type.while_.condition, b->per_type.while_.condition)) return false;
            if (!lists_are_equal(a->per_type.while_.body_statements, b->per_type.while_.body_statements)) return false;
            break;
        case ST_FOR_LOOP:
            if (!expressions_are_equal(a->per_type.for_.init, b->per_type.for_.init)) return false;
            if (!expressions_are_equal(a->per_type.for_.condition, b->per_type.for_.condition)) return false;
            if (!expressions_are_equal(a->per_type.for_.next, b->per_type.for_.next)) return false;
            if (!lists_are_equal(a->per_type.for_.body_statements, b->per_type.for_.body_statements)) return false;
            break;
        case ST_CONTINUE:
            break;
        case ST_BREAK:
            break;
        case ST_RETURN:
            if (!expressions_are_equal(a->per_type.return_.value, b->per_type.return_.value)) return false;
            break;
        case ST_FUNCTION:
            if (!strs_are_equal(a->per_type.function.name, b->per_type.function.name)) return false;
            if (!lists_are_equal(a->per_type.function.arg_names, b->per_type.function.arg_names)) return false;
            if (!lists_are_equal(a->per_type.function.statements, b->per_type.function.statements)) return false;
            break;
    }

    return true;
}

class *statement_class = &(class){
    .classdef_magic = CLASSDEF_MAGIC,
    .type_name = "statement",
    .describe = (describe_func)statement_describe,
    .are_equal = (are_equal_func)statements_are_equal
};

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(statement);
