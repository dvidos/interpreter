#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../utils/failable.h"
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

static char *collect_string_literal(const char *code, int len, int *pos, char opening_quote) {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    char closing_quote = opening_quote;
    
    while ((*pos < len)
        && (code[*pos] != closing_quote)
        && (strlen(buffer) < sizeof(buffer) - 1) 
    ) {
        buffer[strlen(buffer)] = code[*pos];
        (*pos) += 1;
    }

    // since we encountered the closing quote, advance position
    if (code[*pos] == closing_quote)
        (*pos) += 1;

    char *data = malloc(strlen(buffer) + 1);
    strcpy(data, buffer);
    return data;
}

struct char_token_type {
    char *str;
    token_type type;
};

struct char_token_type char_token_types[] = {
    { "(",  T_LPAREN },
    { ")",  T_RPAREN },
    { "[",  T_LSQBRACKET },
    { "]",  T_RSQBRACKET },
    { "+",  T_PLUS },
    { "-",  T_MINUS },
    { "*",  T_ASTERISK },
    { "/",  T_FWD_SLASH },
    { "//", T_DOUBLE_SLASH },
    { ";",  T_SEMICOLON },
    { "~",  T_TIDLE },
    { "!",  T_EXCLAMATION },
    { "!=", T_EXCLAMATION_EQUAL },
    { "=",  T_EQUAL },
    { "==", T_DOUBLE_EQUAL },
    { "->", T_ARROW },
    { ".",  T_DOT },
    { ",",  T_COMMA },
    { "&",  T_AMPERSAND },
    { "&&", T_DOUBLE_AMPERSAND },
    { "|",  T_PIPE },
    { "||", T_DOUBLE_PIPE },
    { "<",  T_SMALLER },
    { "<<", T_DOUBLE_SMALLER },
    { "<=", T_SMALLER_EQUAL },
    { ">",  T_LARGER },
    { ">>", T_DOUBLE_LARGER },
    { ">=", T_LARGER_EQUAL },
};

static token_type get_char_token_type(const char *code, int len, int *pos, char first_char) {

    char second_char = (*pos) + 1 < len ? code[(*pos)+1] : (char)0;
    token_type single_char_type = T_UNKNOWN;
    token_type double_char_type = T_UNKNOWN;

    for (int i = 0; i < (sizeof(char_token_types)/sizeof(char_token_types[0])); i++) {
        char *str = char_token_types[i].str;
        token_type type = char_token_types[i].type;

        if (str[1] != 0 && second_char != 0) {
            if (str[0] == first_char && str[1] == second_char)
                double_char_type = type;
        } else {
            if (str[0] == first_char)
                single_char_type = type;
        }
    }

    // if we matched the double character token, prefer that.
    if (double_char_type != T_UNKNOWN) {
        (*pos) += 1;
        return double_char_type;
    }
    return single_char_type; // can be unknown
}

static failable_token get_token_at_code_position(const char *code, int len, int *pos, token **token_out) {
    if (!skip_whitespace(code, len, pos))
        return ok_token(NULL);
    
    char c = code[*pos];
    (*pos) += 1;

    token_type char_token_type = get_char_token_type(code, len, pos, c);
    if (char_token_type != T_UNKNOWN) {
        return ok_token(new_token(char_token_type));
    }

    if (c == '"' || c == '\'') {
        char *data = collect_string_literal(code, len, pos, c);
        return ok_token(new_data_token(T_STRING_LITERAL, data));
    }

    if (is_number_char(c)) {
        char *data = collect(code, len, pos, c, is_number_char);
        return ok_token(new_data_token(T_NUMBER_LITERAL, data));
    }

    if (is_identifier_char(c)) {
        char *data = collect(code, len, pos, c, is_identifier_char);
        if (strcmp(data, "true") == 0 || strcmp(data, "false") == 0)
            return ok_token(new_data_token(T_BOOLEAN_LITERAL, data));
        else
            return ok_token(new_data_token(T_IDENTIFIER, data));
    }
    
    return failed_token("Unrecognized token at pos %d", *pos);
}

failable_list parse_code_into_tokens(const char *code) {
    list *tokens = new_list();
    if (code == NULL)
        return ok_list(tokens);
    
    token *token;
    int len = strlen(code);
    int pos = 0;
    while (pos < len) {
        failable_token t = get_token_at_code_position(code, len, &pos, &token);
        if (t.failed)
            return failed_list("Didn't get token: %s", t);
        if (t.result == NULL)
            break;
        list_add(tokens, t.result);
    }

    list_add(tokens, new_token(T_END));
    return ok_list(tokens);
}
