#ifndef _TOKEN_H
#define _TOKEN_H

typedef enum token_type {
    T_UNKNOWN,
    T_IDENTIFIER,
    T_NUMBER_LITERAL,
    T_BOOLEAN_LITERAL,
    T_STRING_LITERAL,
    T_LPAREN,
    T_RPAREN,
    T_SEMICOLON,
    T_PLUS,
    T_MINUS,
    T_ASTERISK,
    T_FWD_SLASH,
} token_type;

typedef struct token token;

token *new_token(token_type type);
token *new_token_data(token_type type, const char *data);

token_type token_get_type(token *t);


#endif
