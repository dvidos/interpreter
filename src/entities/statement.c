#include <stdlib.h>
#include <string.h>
#include "../utils/str.h"
#include "../utils/str_builder.h"
#include "../containers/_module.h"
#include "statement.h"


static statement *new_statement(statement_type type, token *token) {
    statement *s = malloc(sizeof(statement));
    s->item_info = expression_item_info;
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
statement *new_try_catch_statement(list *try_statements, const char *exception_identifier, list *catch_statements, list *finally_statements, token *token) {
    statement *s = new_statement(ST_TRY_CATCH, token);
    s->per_type.try_catch.try_statements = try_statements;
    s->per_type.try_catch.exception_identifier = exception_identifier;
    s->per_type.try_catch.catch_statements = catch_statements;
    s->per_type.try_catch.finally_statements = finally_statements;
    return s;
}
statement *new_throw_statement(expression *exception, token *token) {
    statement *s = new_statement(ST_THROW, token);
    s->per_type.throw.exception = exception;
    return s;
}
statement *new_breakpoint_statement(token *token) {
    return new_statement(ST_BREAKPOINT, token);
}


bool statement_is_at(statement *s, const char *filename, int line_no) {
    return s->token != NULL &&
        s->token->line_no == line_no &&
        strcmp(s->token->filename, filename) == 0;
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
        case ST_TRY_CATCH:
            str_builder_add(sb, "try {\n    ");
            list_describe(s->per_type.try_catch.try_statements, "\n    ", sb);
            str_builder_addf(sb, "\n} catch (%s) {\n    ", s->per_type.try_catch.exception_identifier);
            list_describe(s->per_type.try_catch.catch_statements, "\n    ", sb);
            if (s->per_type.try_catch.finally_statements != NULL && !list_empty(s->per_type.try_catch.finally_statements)) {
                str_builder_addf(sb, "\n} finally {\n    ");
                list_describe(s->per_type.try_catch.finally_statements, "\n    ", sb);
            }
            str_builder_add(sb, "\n}");
            break;
        case ST_THROW:
            str_builder_add(sb, "throw (");
            expression_describe(s->per_type.throw.exception, sb);
            str_builder_add(sb, ");");
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
        case ST_TRY_CATCH:
            if (!lists_are_equal(a->per_type.try_catch.try_statements, b->per_type.try_catch.try_statements)) return false;
            if (!strs_are_equal(a->per_type.try_catch.exception_identifier, b->per_type.try_catch.exception_identifier)) return false;
            if (!lists_are_equal(a->per_type.try_catch.catch_statements, b->per_type.try_catch.catch_statements)) return false;
            if (!lists_are_equal(a->per_type.try_catch.finally_statements, b->per_type.try_catch.finally_statements)) return false;
            break;
        case ST_THROW:
            if (!expressions_are_equal(a->per_type.throw.exception, b->per_type.throw.exception)) return false;
            break;
    }

    return true;
}

item_info *statement_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "statement",
    .describe = (describe_item_func)statement_describe,
    .are_equal = (items_equal_func)statements_are_equal
};

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(statement);
