#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "list.h"
#include "expression.h"
#include "token.h"
#include "eval.h"



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

static char *collect(const char *code, int len, int *pos, char_filter_function *filter) {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    buffer[strlen(buffer)] = code[*pos];
    (*pos) += 1;

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
        char *data = collect(code, len, pos, is_number_char);
        return new_data_token(T_NUMBER_LITERAL, data);
    }

    if (is_identifier_char(c)) {
        char *data = collect(code, len, pos, is_identifier_char);
        return new_data_token(T_IDENTIFIER, data);
    }
    
    return new_data_token(T_UNKNOWN, &code[*pos]);
}

static list *parse_string_into_tokens(const char *code) {
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

static list *parse_tokens_into_expressions(list *tokens) {
    // parse the tokens into an AST. Need operation precedence here & double stack.
    return new_list();
}

value *evaluate(const char *code, dict *arguments) {
    list *tokens = parse_string_into_tokens(code);
    list *expressions = parse_tokens_into_expressions(tokens);

    value *result = new_null_value();
    for (int i = 0; i < list_length(expressions); i++) {
        expression *expr = (expression *)list_get(expressions, i);

        // if many expressions, the last result is returned.
        result = execute_expression(expr, arguments);
    }

    return result;
}
