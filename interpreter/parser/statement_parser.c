#include "statement.h"
#include "../lexer/token.h"


void initialize_statement_parser() {
    // anything global 
}

failable_statement parse_statement(iterator *tokens, bool verbose) {
    token *next = tokens->peek(tokens);
    switch (token_get_type(next)) {
        case T_END:
            return ok_statement(NULL);
    }

    return failed("Unknown token type: %d", token_type_str(token_get_type(next)));
}


