#include <string.h>
#include "../parser_expr/expression_parser.h"
#include "statement.h"
#include "../lexer/token.h"
#include "statement_parser.h"

iterator *tokens_it;


void initialize_statement_parser() {
    // anything global 
}

bool accept_token(token_type tt) {
    token *t = tokens_it->curr(tokens_it);
    if (token_get_type(t) != tt)
        return false;
    tokens_it->next(tokens_it);
    return true;
}
bool accept_identifier(const char *data) {
    token *t = tokens_it->curr(tokens_it);
    if (token_get_type(t) != T_IDENTIFIER)
        return false;
    if (strcmp(token_get_data(t), data) != 0)
        return false;
    tokens_it->next(tokens_it);
    return true;
}

static failable_statement parse_if_statement() {
    if (!accept_identifier("if")) return failed_statement("was expecting 'if'");
    if (!accept_token(T_LPAREN))  return failed_statement("was expecting '('");
    
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    if (!accept_token(T_LPAREN)) return failed_statement("was expecting ')'");

    failable_list body_parsing = parse_statements_block(tokens_it);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    bool has_else = false;
    list *else_statements = NULL;

    if (accept_identifier("else")) {
        failable_list else_parsing = parse_statements_block(tokens_it);
        if (else_parsing.failed) return failed_statement("%s", else_parsing.err_msg);
        else_statements = else_parsing.result;
    }

    return ok_statement(new_if_statement(condition, body_statements, has_else, else_statements));
}

static failable_statement parse_while_statement() {
    if (!accept_identifier("while")) return failed_statement("was expecting 'while'");
    if (!accept_token(T_LPAREN))     return failed_statement("was expecting '('");
    
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    if (!accept_token(T_LPAREN)) return failed_statement("was expecting ')'");

    failable_list body_parsing = parse_statements_block(tokens_it);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_while_statement(condition, body_statements));
}

static failable_statement parse_for_statement() {
    if (!accept_identifier("while")) return failed_statement("was expecting 'while'");
    if (!accept_token(T_LPAREN))     return failed_statement("was expecting '('");
    
    failable_expression expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement("cannot parse init: %s", expr_parsing.err_msg);
    expression *init = expr_parsing.result;

    expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *cond = expr_parsing.result;

    expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *next = expr_parsing.result;

    if (!accept_token(T_LPAREN)) return failed_statement("was expecting ')'");

    failable_list body_parsing = parse_statements_block(tokens_it);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_for_statement(init, cond, next, body_statements));
}

static failable_statement parse_break_statement() {
    return ok_statement(new_break_statement());
}

static failable_statement parse_continue_statement() {
    return ok_statement(new_continue_statement());
}

static failable_statement parse_expression_statement() {
    failable_expression parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (parsing.failed) return failed_statement("%s", parsing.err_msg);
    return ok_statement(new_expression_statement(parsing.result));
}

failable_statement parse_statement(iterator *tokens) {
    tokens_it = tokens;

    token *next = tokens_it->peek(tokens);
    switch (token_get_type(next)) {
        case T_IDENTIFIER:
            const char *name = token_get_data(next);
            if (strcmp(name, "if") == 0) {
                return parse_if_statement();
            } else if (strcmp(name, "while") == 0) {
                return parse_while_statement();
            } else if (strcmp(name, "for") == 0) {
                return parse_for_statement();
            } else if (strcmp(name, "break") == 0) {
                return parse_break_statement();
            } else if (strcmp(name, "continue") == 0) {
                return parse_continue_statement();
            } else {
                return parse_expression_statement();
            }
            break;
    }

    return failed_statement("Unknown token type: %s", token_type_str(token_get_type(next)));
}

failable_list parse_statements_block(iterator *tokens) {
    tokens_it = tokens;

    list *statements = new_list(containing_statements);
    failable_statement parsing;

    if (accept_token(T_LBRACKET)) {
        token_type next_type = token_get_type(tokens_it->peek(tokens_it));
        while (next_type != T_RBRACKET && next_type != T_END) {
            parsing = parse_statement(tokens_it);
            if (parsing.failed) return failed_list("%s", parsing.err_msg);
            list_add(statements, parsing.result);
            
            next_type = token_get_type(tokens_it->peek(tokens_it));
        }
    } else {
        parsing = parse_statement(tokens_it);
        if (parsing.failed) return failed_list("%s", parsing.err_msg);
        list_add(statements, parsing.result);
    }

    return ok_list(statements);
}
