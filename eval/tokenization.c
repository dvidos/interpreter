#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tokenization.h"
#include "token.h"


static bool is_whilespace(char c) {
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

static bool is_identifier_char(char c) {
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_'));
}

static bool is_number_char(char c) {
    return ((c >= '0' && c <= '9') || (c == '.'));
}

typedef bool char_filter_function(char c);

static bool skip_whitespace(const char *code, int len, int *pos) {
    while ((*pos < len) && is_whilespace(code[*pos]))
        (*pos) += 1;
    
    return (*pos < len);
}

static char *collect(const char *code, int len, int *pos, char first_char, char_filter_function *filter) {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = first_char;

    while ((*pos < len)
        && (filter(code[*pos]))
        && (strlen(buffer) < sizeof(buffer) - 1) 
    ) {
        buffer[strlen(buffer)] = code[*pos];
        (*pos) += 1;
    }

    char *data = malloc(strlen(buffer) + 1);
    strcpy(data, buffer);
    return data;
}

static token *get_token_at_code_position(const char *code, int len, int *pos) {
    if (!skip_whitespace(code, len, pos))
        return NULL;
    
    char c = code[*pos];
    (*pos) += 1;

    switch (c) {
        case '(': return new_token(T_LPAREN);
        case ')': return new_token(T_RPAREN);
        case '+': return new_token(T_PLUS);
        case '-': return new_token(T_MINUS);
        case '*': return new_token(T_ASTERISK);
        case '/': return new_token(T_FWD_SLASH);
        case ';': return new_token(T_SEMICOLON);
    }

    if (is_number_char(c)) {
        char *data = collect(code, len, pos, c, is_number_char);
        return new_data_token(T_NUMBER_LITERAL, data);
    }

    if (is_identifier_char(c)) {
        char *data = collect(code, len, pos, c, is_identifier_char);
        return new_data_token(T_IDENTIFIER, data);
    }
    
    return new_data_token(T_UNKNOWN, &code[*pos]);
}

list *parse_code_into_tokens(const char *code) {
    list *tokens = new_list();
    if (code == NULL)
        return tokens;
    int len = strlen(code);
    int pos = 0;
    while (pos < len) {
        token *t = get_token_at_code_position(code, len, &pos);
        if (t == NULL)
            break;
        list_add(tokens, t);
        if (token_get_type(t) == T_UNKNOWN)
            break;
    }
    return tokens;
}
