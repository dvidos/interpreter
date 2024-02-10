#include <string.h>
#include "../utils/str.h"
#include "../lexer/_module.h"
#include "expression_parser.h"
#include "statement_parser.h"
#include "../entities/statement.h"

static iterator *tokens_it;
static token *last_accepted_token;

void initialize_statement_parser() {
    // anything global 
    last_accepted_token = NULL;
}

static bool accept(token_type tt) {
    token *t = tokens_it->curr(tokens_it);
    if (t->type != tt)
        return false;
    last_accepted_token = t;
    tokens_it->next(tokens_it);
    return true;
}

static token *peek() {
    return tokens_it->curr(tokens_it);
}

static inline token *accepted() {
    return last_accepted_token;
}

static bool tokens_finished() {
    return 
        (tokens_it->valid(tokens_it) == false) ||
        (((token *)(tokens_it->curr(tokens_it)))->type == T_END);
}

static failable_statement parse_if_statement() {
    if (!accept(T_IF)) return failed_statement(NULL, "was expecting 'if'");
    token *token = accepted();
    if (!accept(T_LPAREN))  return failed_statement(NULL, "was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement(&expr_parsing, "cannot parse condition");
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement(&body_parsing, NULL);
    list *body_statements = body_parsing.result;

    bool has_else = false;
    list *else_statements = NULL;

    if (accept(T_ELSE)) {
        has_else = true;
        failable_list else_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
        if (else_parsing.failed) return failed_statement(&else_parsing, NULL);
        else_statements = else_parsing.result;
    }

    return ok_statement(new_if_statement(condition, body_statements, has_else, else_statements, token));
}

static failable_statement parse_while_statement() {
    if (!accept(T_WHILE)) return failed_statement(NULL, "was expecting 'while'");
    token *token = accepted();
    if (!accept(T_LPAREN))     return failed_statement(NULL, "was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement(&expr_parsing, "cannot parse condition");
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement(&body_parsing, NULL);
    list *body_statements = body_parsing.result;

    return ok_statement(new_while_statement(condition, body_statements, token));
}

static failable_statement parse_for_statement() {
    if (!accept(T_FOR)) return failed_statement(NULL, "was expecting 'for'");
    token *token = accepted();
    if (!accept(T_LPAREN))   return failed_statement(NULL, "was expecting '('");
    
    failable_expression expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement(&expr_parsing, "cannot parse init");
    expression *init = expr_parsing.result;

    expr_parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
    if (expr_parsing.failed) return failed_statement(&expr_parsing, "cannot parse condition");
    expression *cond = expr_parsing.result;

    // expression parsing consumes RPAREN as well.
    expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement(&expr_parsing, "cannot parse condition");
    expression *next = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement(&body_parsing, NULL);
    list *body_statements = body_parsing.result;

    return ok_statement(new_for_statement(init, cond, next, body_statements, token));
}

static failable_statement parse_break_statement() {
    if (!accept(T_BREAK)) return failed_statement(NULL, "was expecting 'break'");
    token *token = accepted();
    if (!accept(T_SEMICOLON)) return failed_statement(NULL, "was expecting ';'");
    return ok_statement(new_break_statement(token));
}

static failable_statement parse_continue_statement() {
    if (!accept(T_CONTINUE)) return failed_statement(NULL, "was expecting 'continue'");
    token *token = accepted();
    if (!accept(T_SEMICOLON)) return failed_statement(NULL, "was expecting ';'");
    return ok_statement(new_continue_statement(token));
}

static failable_statement parse_expression_statement() {
    failable_expression parsing = parse_expression(tokens_it, CM_SEMICOLON_OR_END, false);
    if (parsing.failed) return failed_statement(&parsing, NULL);
    return ok_statement(new_expression_statement(parsing.result));
}

static failable_statement parse_return_statement() {
    if (!accept(T_RETURN)) return failed_statement(NULL, "was expecting 'return'");
    token *token = accepted();

    failable_expression parsing;
    expression *return_value_expression;

    if (accept(T_SEMICOLON)) {
        return_value_expression = NULL;
    } else if (accept(T_LPAREN)) {
        parsing = parse_expression(tokens_it, CM_RPAREN, false);
        if (parsing.failed) return failed_statement(&parsing, "cannot parse value");
        return_value_expression = parsing.result;
        if (!accept(T_SEMICOLON)) return failed_statement(NULL, "was expecting ';'");
    } else {
        parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
        if (parsing.failed) return failed_statement(&parsing, "cannot parse value");
        return_value_expression = parsing.result;
    }

    return ok_statement(new_return_statement(return_value_expression, token));
}

static failable_statement parse_function_statement() {
    if (!accept(T_FUNCTION_KEYWORD)) return failed_statement(NULL, "was expecting 'function'");
    token *token = accepted();

    // function name is optional
    const char *name = NULL;
    if (accept(T_IDENTIFIER))
        name = accepted()->data;
    
    list *arg_names = new_list(str_class);
    if (!accept(T_LPAREN))
        return failed_statement(NULL, "Was expecting arguments list after function");
    while (!accept(T_RPAREN)) {
        if (!accept(T_IDENTIFIER))
            return failed_statement(NULL, "Was expecting identifier in function arg names");
        list_add(arg_names, (void *)accepted()->data); // we lose const here
        accept(T_COMMA);
    }

    failable_list stmts = parse_statements(tokens_it, SP_BLOCK_MANDATORY);
    if (stmts.failed) return failed_statement(&stmts, "Parsing function body");

    return ok_statement(new_function_statement(name, arg_names, stmts.result, token));
}

static failable_statement parse_breakpoint_statement() {
    if (!accept(T_BREAKPOINT)) return failed_statement(NULL, "was expecting 'breakpoint'");
    token *token = accepted();
    if (!accept(T_SEMICOLON)) return failed_statement(NULL, "was expecting ';'");
    return ok_statement(new_breakpoint_statement(token));
}

failable_statement parse_statement(iterator *tokens) {
    tokens_it = tokens;

    token_type tt = peek()->type;
    switch (tt) {
        case T_IF:               return parse_if_statement();
        case T_WHILE:            return parse_while_statement();
        case T_FOR:              return parse_for_statement();
        case T_BREAK:            return parse_break_statement();
        case T_CONTINUE:         return parse_continue_statement();
        case T_RETURN:           return parse_return_statement();
        case T_FUNCTION_KEYWORD: return parse_function_statement();
        case T_BREAKPOINT:       return parse_breakpoint_statement();
        default:                 return parse_expression_statement();
    }
}

failable_list parse_statements(iterator *tokens, statement_parsing_mode mode) {
    tokens_it = tokens;

    // use cases:
    // - many statements without brackets, e.g. a whole script file
    // - many statements in brackets block '{ ... }' 
    // - single statement without brackets, e.g. after an "if"
    // - many statements with a block, e.g. a function body

    list *statements = new_list(statement_class);
    bool is_block = false;
    bool done = false;
    
    if (mode == SP_BLOCK_MANDATORY) {
        if (!accept(T_LBRACKET))
            return failed_list(NULL, "Was expecting '{'");
    } else if (mode == SP_SINGLE_OR_BLOCK) {
        is_block = accept(T_LBRACKET);
    }

    while (true) {
        // checking first allows for empty blocks
        if (mode == SP_BLOCK_MANDATORY)
            done = accept(T_RBRACKET);
        else if (mode == SP_SINGLE_OR_BLOCK)
            done = is_block ? accept(T_RBRACKET) : list_length(statements) > 0;
        else if (mode == SP_SEQUENTIAL_STATEMENTS)
            done = tokens_finished();
        else
            done = true;
        if (done)
            break;

        failable_statement parsing = parse_statement(tokens_it);
        if (parsing.failed) return failed_list(&parsing, NULL);
        list_add(statements, parsing.result);
    }

    return ok_list(statements);
}
