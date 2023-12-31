#include <stdlib.h>
#include <string.h>
#include "token.h"

struct token {
    token_type type;
    const char *data; // e.g. identifier or number
};

token *new_token(token_type type) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = NULL;
    return t;
}

token *new_data_token(token_type type, const char *data) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = data;
    return t;
}

token_type inline token_get_type(token *t) {
    return t->type;
}

inline const char *token_get_data(token *t) {
    return t->data;
}

const char *token_type_str(token_type type) {
    switch (type) {
        case T_UNKNOWN: return "T_UNKNOWN";
        case T_IDENTIFIER: return "T_IDENTIFIER";
        case T_NUMBER_LITERAL: return "T_NUMBER_LITERAL";
        case T_STRING_LITERAL: return "T_STRING_LITERAL";
        case T_BOOLEAN_LITERAL: return "T_BOOLEAN_LITERAL";
        case T_LPAREN: return "T_LPAREN";
        case T_RPAREN: return "T_RPAREN";
        case T_LSQBRACKET: return "T_LSQBRACKET";
        case T_RSQBRACKET: return "T_RSQBRACKET";
        case T_SEMICOLON: return "T_SEMICOLON";
        case T_PLUS: return "T_PLUS";
        case T_MINUS: return "T_MINUS";
        case T_ASTERISK: return "T_ASTERISK";
        case T_FWD_SLASH: return "T_FWD_SLASH";
        case T_DOUBLE_SLASH: return "T_DOUBLE_SLASH";
        case T_PIPE: return "T_PIPE";
        case T_DOUBLE_PIPE: return "T_DOUBLE_PIPE";
        case T_AMPERSAND: return "T_AMPERSAND";
        case T_DOUBLE_AMPERSAND: return "T_DOUBLE_AMPERSAND";
        case T_TIDLE: return "T_TIDLE";
        case T_EXCLAMATION: return "T_EXCLAMATION";
        case T_EXCLAMATION_EQUAL: return "T_EXCLAMATION_EQUAL";
        case T_EQUAL: return "T_EQUAL";
        case T_DOUBLE_EQUAL: return "T_DOUBLE_EQUAL";
        case T_LARGER_EQUAL: return "T_LARGER_EQUAL";
        case T_LARGER: return "T_LARGER";
        case T_DOUBLE_LARGER: return "T_DOUBLE_LARGER";
        case T_SMALLER_EQUAL: return "T_SMALLER_EQUAL";
        case T_SMALLER: return "T_SMALLER";
        case T_DOUBLE_SMALLER: return "T_DOUBLE_SMALLER";
        case T_ARROW: return "T_ARROW";
        case T_DOT: return "T_DOT";
        case T_COMMA: return "T_COMMA";
        case T_END: return "T_END";
    }

    return "(unknown type)";
}

void token_print(token *t, FILE *stream, char *prefix) {
    token_type tt = token_get_type(t);
    const char *data = token_get_data(t);
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);

    fprintf(stream, "%s%s%s%s%s\n", 
        prefix,
        token_type_str(tt), 
        has_data ? " \"" : "",
        has_data ? data : "",
        has_data ? "\"" : ""
    );
}

void token_print_list(list *tokens, FILE *stream, char *prefix) {
    iterator *it;
    for (it = list_iterator(tokens); iterator_valid(it); it = iterator_next(it))
        token_print((token *)iterator_current(it), stream, prefix);
}

bool tokens_are_equal(token *a, token *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->type != b->type)
        return false;
    if (strcmp(a->data, b->data) != 0)
        return false;

    return true;
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(token);
