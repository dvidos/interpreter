#include <string.h>
#include "statement.h"
#include "../lexer/token.h"


void initialize_statement_parser() {
    // anything global 
}

static failable_statement parse_if_statement() {
    return failed_statement("not implemented");
}

static failable_statement parse_while_statement() {
    return failed_statement("not implemented");
}

static failable_statement parse_for_statement() {
    return failed_statement("not implemented");
}

static failable_statement parse_break_statement() {
    return ok_statement(new_break_statement());
}

static failable_statement parse_continue_statement() {
    return ok_statement(new_continue_statement());
}

static failable_statement parse_expression_statement() {
    
}

failable_statement parse_statement(iterator *tokens, bool verbose) {
    token *next = tokens->peek(tokens);
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


