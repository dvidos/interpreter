#include <string.h>
#include "../lexer/_module.h"
#include "expression_parser.h"
#include "statement_parser.h"
#include "../entities/statement.h"

static iterator *tokens_it;
static token *_last_accepted_token;

void initialize_statement_parser() {
    // anything global 
}


static bool accept(token_type tt) {
    token *t = tokens_it->curr(tokens_it);
    if (token_get_type(t) != tt)
        return false;
    _last_accepted_token = t;
    tokens_it->next(tokens_it);
    return true;
}
static token *peek() {
    return tokens_it->curr(tokens_it);
}
static inline token *accepted() {
    return _last_accepted_token;
}

static bool tokens_finished() {
    return 
        (tokens_it->valid(tokens_it) == false) ||
        (token_get_type(tokens_it->curr(tokens_it)) == T_END);
}

static failable_statement parse_if_statement() {
    if (!accept(T_IF)) return failed_statement("was expecting 'if'");
    if (!accept(T_LPAREN))  return failed_statement("was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    bool has_else = false;
    list *else_statements = NULL;

    if (accept(T_ELSE)) {
        has_else = true;
        failable_list else_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
        if (else_parsing.failed) return failed_statement("%s", else_parsing.err_msg);
        else_statements = else_parsing.result;
    }

    return ok_statement(new_if_statement(condition, body_statements, has_else, else_statements));
}

static failable_statement parse_while_statement() {
    if (!accept(T_WHILE)) return failed_statement("was expecting 'while'");
    if (!accept(T_LPAREN))     return failed_statement("was expecting '('");
    
    // expression parsing consumes RPAREN as well.
    failable_expression expr_parsing = parse_expression(tokens_it, CM_RPAREN, false);
    if (expr_parsing.failed) return failed_statement("cannot parse condition: %s", expr_parsing.err_msg);
    expression *condition = expr_parsing.result;

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_while_statement(condition, body_statements));
}

static failable_statement parse_for_statement() {
    if (!accept(T_FOR)) return failed_statement("was expecting 'for'");
    if (!accept(T_LPAREN))   return failed_statement("was expecting '('");
    
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

    failable_list body_parsing = parse_statements(tokens_it, SP_SINGLE_OR_BLOCK);
    if (body_parsing.failed) return failed_statement("%s", body_parsing.err_msg);
    list *body_statements = body_parsing.result;

    return ok_statement(new_for_statement(init, cond, next, body_statements));
}

static failable_statement parse_break_statement() {
    if (!accept(T_BREAK)) return failed_statement("was expecting 'break'");
    if (!accept(T_SEMICOLON)) return failed_statement("was expecting ';'");
    return ok_statement(new_break_statement());
}

static failable_statement parse_continue_statement() {
    if (!accept(T_CONTINUE)) return failed_statement("was expecting 'continue'");
    if (!accept(T_SEMICOLON)) return failed_statement("was expecting ';'");
    return ok_statement(new_continue_statement());
}

static failable_statement parse_expression_statement() {
    failable_expression parsing = parse_expression(tokens_it, CM_SEMICOLON_OR_END, false);
    if (parsing.failed) return failed_statement("%s", parsing.err_msg);
    return ok_statement(new_expression_statement(parsing.result));
}

static failable_statement parse_return_statement() {
    if (!accept(T_RETURN)) return failed_statement("was expecting 'return'");

    failable_expression parsing;
    expression *return_value_expression;

    if (accept(T_LPAREN)) {
        parsing = parse_expression(tokens_it, CM_RPAREN, false);
        if (parsing.failed) return failed_statement("cannot parse value: %s", parsing.err_msg);
        return_value_expression = parsing.result;
        if (!accept(T_SEMICOLON)) return failed_statement("was expecting ';'");

    } else if (accept(T_SEMICOLON)) {
        return_value_expression = NULL;
    } else {
        parsing = parse_expression(tokens_it, CM_SEMICOLON, false);
        if (parsing.failed) return failed_statement("cannot parse value: %s", parsing.err_msg);
        return_value_expression = parsing.result;
    }

    return ok_statement(new_return_statement(return_value_expression));
}

static failable_list parse_function_arguments() {
    list *arg_names = new_list(containing_strs);
    while (true) {
        if (accept(T_RPAREN))
            break;
        
        token *t = peek();
        if (token_get_type(t) != T_IDENTIFIER)
            return failed_list("Was expecting identifier in function arg names");
        accept(T_IDENTIFIER);
        list_add(arg_names, (void *)token_get_data(t)); // we lose const here

        if (accept(T_COMMA))
            ;
    }
    return ok_list(arg_names);
}

static failable_statement parse_function_statement() {
    if (!accept(T_FUNCTION_KEYWORD)) return failed_statement("was expecting 'function'");

    // allowing anonymous functions, as in javascript
    const char *name = NULL;
    token *name_token = peek();
    if (token_get_type(name_token) == T_IDENTIFIER) {
        name = token_get_data(name_token);
        accept(token_get_type(name_token));
    }
    
    if (!accept(T_LPAREN))
        return failed_statement("was expecting arguments list after function");
    
    failable_list arg_names = parse_function_arguments();
    if (arg_names.failed) return failed_statement("Parsing function arguements: %s", arg_names.err_msg);

    failable_list stmts = parse_statements(tokens_it, SP_BLOCK_MANDATORY);
    if (stmts.failed) return failed_statement("Parsing function body: %s", stmts.err_msg);

    return ok_statement(new_function_statement(name, arg_names.result, stmts.result));
}

failable_statement parse_statement(iterator *tokens) {
    tokens_it = tokens;

    token_type tt = token_get_type(peek());
    switch (tt) {
        case T_IF:               return parse_if_statement();
        case T_WHILE:            return parse_while_statement();
        case T_FOR:              return parse_for_statement();
        case T_BREAK:            return parse_break_statement();
        case T_CONTINUE:         return parse_continue_statement();
        case T_RETURN:           return parse_return_statement();
        case T_FUNCTION_KEYWORD: return parse_function_statement();
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

    list *statements = new_list(containing_statements);
    bool is_block = false;
    bool done = false;
    
    if (mode == SP_BLOCK_MANDATORY) {
        if (!accept(T_LBRACKET))
            return failed_list("Was expecting '{'");
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
        if (parsing.failed) return failed_list("%s", parsing.err_msg);
        list_add(statements, parsing.result);
    }

    return ok_list(statements);
}
