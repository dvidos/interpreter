#include <string.h>
#include "../parser_expr/expression_parser.h"
#include "statement.h"
#include "../lexer/token.h"
#include "statement_parser.h"

iterator *tokens_it;


void initialize_statement_parser() {
    // anything global 
}

static bool accept_token(token_type tt) {
    token *t = tokens_it->curr(tokens_it);
    if (token_get_type(t) != tt)
        return false;
    tokens_it->next(tokens_it);
    return true;
}
static bool tokens_finished() {
    return 
        (tokens_it->valid(tokens_it) == false) ||
        (token_get_type(tokens_it->curr(tokens_it)) == T_END);
}

static failable_statement parse_if_statement() {
    if (!accept_token(T_IF)) return failed_statement("was expecting 'if'");
    if (!accept_token(T_LPAREN))  return failed_statement("was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, true);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    bool has_else = false;
    list *else_statements = NULL;

    if (accept_token(T_ELSE)) {
        has_else = true;
        failable_list else_parsing = parse_statements(tokens_it, true);
        if (else_parsing.failed) return failed_statement("%s", else_parsing.err_msg);
        else_statements = else_parsing.result;
    }

    return ok_statement(new_if_statement(condition, body_statements, has_else, else_statements));
}

static failable_statement parse_while_statement() {
    if (!accept_token(T_WHILE)) return failed_statement("was expecting 'while'");
    if (!accept_token(T_LPAREN))     return failed_statement("was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, true);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_while_statement(condition, body_statements));
}

static failable_statement parse_for_statement() {
    if (!accept_token(T_FOR)) return failed_statement("was expecting 'for'");
    if (!accept_token(T_LPAREN))   return failed_statement("was expecting '('");
    
    failable_expression expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement("cannot parse init: %s", expr_parsing.err_msg);
    expression *init = expr_parsing.result;

    expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *cond = expr_parsing.result;

    // expression parsing consumes RPAREN as well.
    expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *next = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, true);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_for_statement(init, cond, next, body_statements));
}

static failable_statement parse_break_statement() {
    if (!accept_token(T_BREAK)) return failed_statement("was expecting 'break'");
    if (!accept_token(T_SEMICOLON)) return failed_statement("was expecting ';'");
    return ok_statement(new_break_statement());
}

static failable_statement parse_continue_statement() {
    if (!accept_token(T_CONTINUE)) return failed_statement("was expecting 'continue'");
    if (!accept_token(T_SEMICOLON)) return failed_statement("was expecting ';'");
    return ok_statement(new_continue_statement());
}

static failable_statement parse_expression_statement() {
    failable_expression parsing = parse_expression(tokens_it, CM_SEMICOLON_OR_END, false);
    if (parsing.failed) return failed_statement("%s", parsing.err_msg);
    return ok_statement(new_expression_statement(parsing.result));
}

static failable_statement parse_return_statement() {
    if (!accept_token(T_RETURN)) return failed_statement("was expecting 'return'");

    failable_expression parsing;
    expression *return_value_expression;

    if (accept_token(T_LPAREN)) {
        parsing = parse_expression(tokens_it, CM_RPAREN, false);
        if (parsing.failed) return failed_statement("cannot parse value: %s", parsing.err_msg);
        return_value_expression = parsing.result;
        if (!accept_token(T_SEMICOLON)) return failed_statement("was expecting ';'");

    } else if (accept_token(T_SEMICOLON)) {
        return_value_expression = NULL;
    } else {
        parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
        if (parsing.failed) return failed_statement("cannot parse value: %s", parsing.err_msg);
        return_value_expression = parsing.result;
    }

    return ok_statement(new_return_statement(return_value_expression));
}

failable_statement parse_statement(iterator *tokens) {
    tokens_it = tokens;

    token *t = tokens_it->curr(tokens);
    switch (token_get_type(t)) {
        case T_IF:       return parse_if_statement();
        case T_WHILE:    return parse_while_statement();
        case T_FOR:      return parse_for_statement();
        case T_BREAK:    return parse_break_statement();
        case T_CONTINUE: return parse_continue_statement();
        case T_RETURN:   return parse_return_statement();
        default:         return parse_expression_statement();
    }

    return failed_statement("Unknown token type: %s", token_type_str(token_get_type(t)));
}

failable_list parse_statements(iterator *tokens, bool single_statement_unless_block) {
    tokens_it = tokens;

    list *statements = new_list(containing_statements);
    failable_statement parsing;
    
    // three use cases:
    // - many statements without brackets, e.g. a whole script file
    // - many statements in brackets block '{ ... }' 
    // - single statement without brackets, e.g. after an "if"

    bool in_block = accept_token(T_LBRACKET);
    bool done = false;
    while (!done) {
        parsing = parse_statement(tokens_it);
        if (parsing.failed) return failed_list("%s", parsing.err_msg);
        list_add(statements, parsing.result);
        
        if (in_block)
            done = accept_token(T_RBRACKET);
        else
            done = single_statement_unless_block ? true : tokens_finished();
    }

    return ok_list(statements);
}
