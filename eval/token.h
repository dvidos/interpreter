#ifndef _TOKEN_H
#define _TOKEN_H

#include "../utils/failable.h"

typedef enum token_type {
    T_UNKNOWN,

    T_IDENTIFIER,
    T_NUMBER_LITERAL,
    T_STRING_LITERAL,
    T_BOOLEAN_LITERAL,

    T_LPAREN,
    T_RPAREN,
    T_LSQBRACKET,
    T_RSQBRACKET,
    T_SEMICOLON,
    T_PLUS,
    T_MINUS,
    T_ASTERISK,
    T_FWD_SLASH,
    T_DOUBLE_SLASH,
    T_PIPE,
    T_DOUBLE_PIPE,
    T_AMPERSAND,
    T_DOUBLE_AMPERSAND,
    T_TIDLE,
    T_EXCLAMATION,
    T_EXCLAMATION_EQUAL,
    T_EQUAL,
    T_DOUBLE_EQUAL,
    T_LARGER_EQUAL,
    T_LARGER,
    T_DOUBLE_LARGER,
    T_SMALLER_EQUAL,
    T_SMALLER,
    T_DOUBLE_SMALLER,
    T_ARROW,  // "->"
    T_DOT,
    T_COMMA,

    T_END // the last token of the stream
} token_type;

typedef struct token token;

token *new_token(token_type type);
token *new_data_token(token_type type, const char *data);

token_type token_get_type(token *t);
const char *token_get_data(token *t);

STRONGLY_TYPED_FAILABLE_DECLARATION(token);

#endif
