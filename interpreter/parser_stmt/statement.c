#include <stdlib.h>
#include "../../utils/strbld.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/contained_item.h"
#include "statement.h"


struct statement {
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
    } per_type;
};

// ----------------------------------

statement *new_expression_statement(expression *expr) {
    statement *s = malloc(sizeof(statement));
    s->type = ST_EXPRESSION;
    s->per_type.expr.expr = expr;
    return s;
}
statement *new_if_statement(expression *condition, list *body_statements, bool has_else, list *else_body_statements) {
    statement *s = malloc(sizeof(statement));
    s->type = ST_IF;
    s->per_type.if_.condition = condition;
    s->per_type.if_.body_statements = body_statements;
    s->per_type.if_.has_else = has_else;
    s->per_type.if_.else_body_statements = else_body_statements;
    return s;
}
statement *new_while_statement(expression *condition, list *body_statements) {
    statement *s = malloc(sizeof(statement));
    s->type = ST_WHILE;
    s->per_type.while_.condition = condition;
    s->per_type.while_.body_statements = body_statements;
    return s;
}
statement *new_for_statement(expression *init, expression *condition, expression *next, list *body_statements) {
    statement *s = malloc(sizeof(statement));
    s->type = ST_FOR_LOOP;
    s->per_type.for_.init = init;
    s->per_type.for_.condition = condition;
    s->per_type.for_.next = next;
    s->per_type.for_.body_statements = body_statements;
    return s;
}
statement *new_break_statement() {
    statement *s = malloc(sizeof(statement));
    s->type = ST_BREAK;
    return s;
}
statement *new_continue_statement() {
    statement *s = malloc(sizeof(statement));
    s->type = ST_CONTINUE;
    return s;
}
statement *new_return_statement(expression *value) {
    statement *s = malloc(sizeof(statement));
    s->type = ST_RETURN;
    s->per_type.return_.value = value;
    return s;
}


statement_type statement_get_type(statement *s) {
    return s->type;
}

expression *statement_get_expression(statement *s, int mnemonic) {
    switch (s->type) {
        case ST_IF:
            return s->per_type.if_.condition;
        case ST_WHILE:
            return s->per_type.while_.condition;
        case ST_FOR_LOOP:
            if      (mnemonic == 0) return s->per_type.for_.init;
            else if (mnemonic == 1) return s->per_type.for_.condition;
            else if (mnemonic == 2) return s->per_type.for_.next;
        case ST_RETURN:
            return s->per_type.return_.value;
        case ST_EXPRESSION:
            return s->per_type.expr.expr;
    }
    return NULL;
}

bool statement_has_alternate_body(statement *s) {
    return s->type == ST_IF && s->per_type.if_.has_else;
}

list *statement_get_statements_body(statement *s, bool alternative) {
    switch (s->type) {
        case ST_IF:
            return alternative ? s->per_type.if_.else_body_statements : s->per_type.if_.body_statements;
        case ST_WHILE:
            return s->per_type.while_.body_statements;
        case ST_FOR_LOOP:
            return s->per_type.for_.body_statements;
    }
    return NULL;
}

const char *statement_to_string(statement *s) {
    strbld *sb = new_strbld();

    switch (s->type) {
        case ST_EXPRESSION:
            strbld_cat(sb, "expr(");
            strbld_cat(sb, expression_to_string(s->per_type.expr.expr));
            strbld_cat(sb, ")");
            break;
        case ST_IF:
            strbld_catf(sb, "if (%s) {\n", expression_to_string(s->per_type.if_.condition));
            strbld_cat(sb, list_to_string(s->per_type.if_.body_statements, "\n"));
            if (s->per_type.if_.has_else) {
                strbld_cat(sb, "\n} else {\n");
                strbld_cat(sb, list_to_string(s->per_type.if_.else_body_statements, "\n"));
            }
            strbld_cat(sb, "\n}");
            break;
        case ST_WHILE:
            strbld_catf(sb, "while (%s) {\n", expression_to_string(s->per_type.while_.condition));
            strbld_cat(sb, list_to_string(s->per_type.while_.body_statements, "\n"));
            strbld_cat(sb, "\n}");
            break;
        case ST_FOR_LOOP:
            strbld_catf(sb, "for (%s; %s; %s) {\n",
                expression_to_string(s->per_type.for_.init),
                expression_to_string(s->per_type.for_.condition),
                expression_to_string(s->per_type.for_.next));
            strbld_cat(sb, list_to_string(s->per_type.for_.body_statements, "\n"));
            strbld_cat(sb, "\n}");
            break;
        case ST_CONTINUE:
            strbld_cat(sb, "continue;");
            break;
        case ST_BREAK:
            strbld_cat(sb, "break;");
            break;
    }

    return strbld_charptr(sb);
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
    }

    return true;
}

contained_item *containing_statements = &(contained_item){
    .type_name = "statement",
    .to_string = (to_string_func)statement_to_string,
    .are_equal = (are_equal_func)statements_are_equal
};

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(statement);


