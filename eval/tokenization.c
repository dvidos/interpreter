#include <stdio.h>
#include <stdarg.h>
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

    char second_char = (*pos) < len ? code[(*pos)] : (char)0;
    token_type single_char_type = T_UNKNOWN;
    token_type double_char_type = T_UNKNOWN;

    for (int i = 0; i < (sizeof(char_token_types)/sizeof(char_token_types[0])); i++) {
        char *str = char_token_types[i].str;
        token_type type = char_token_types[i].type;

        if (str[1] != 0 && second_char != 0) {
            if (str[0] == first_char && str[1] == second_char)
                double_char_type = type;
        } else {
            if (str[0] == first_char && str[1] == 0)
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
    
    return failed_token("Unrecognized token at pos %d ('%c')", *pos, code[*pos]);
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
            return failed_list("Cannot get token: %s", t);
        if (t.result == NULL)
            break;
        list_add(tokens, t.result);
    }

    list_add(tokens, new_token(T_END));
    return ok_list(tokens);
}



// -----------------------------------

// for each token, pass in expected type. for identifiers and literals, pass in data.
static bool use_case_passes(const char *code, bool expect_failure, int expected_tokens, ...) {
    failable_list parsing = parse_code_into_tokens(code);

    // test failure
    if (expect_failure) {
        if (!parsing.failed) {
            fprintf(stderr, "Parsing did not fail as expected: code=\"%s\")\n", code);
            return false;
        }
        return true;
    }

    // success, verify
    if (parsing.failed) {
        fprintf(stderr, "Parsing failed unexpectedly: code=\"%s\")\n", code);
        return false;
    }

    list *tokens = parsing.result;
    if (tokens == NULL) {
        fprintf(stderr, "Parsing returned NULL list: code=\"%s\")\n", code);
        return false;
    }

    if (list_length(tokens) != expected_tokens) {
        fprintf(stderr, "Expected %d tokens, gotten %d, code=\"%s\")\n", expected_tokens, list_length(tokens), code);
        token_print_list(tokens, stderr, "  - ");
        return false;
    }

    va_list args;
    va_start(args, expected_tokens);
    for (int i = 0; i < expected_tokens; i++) {
        token *t = list_get(tokens, i);

        token_type expected_type = va_arg(args, token_type);
        token_type actual_type = token_get_type(t);
        if (actual_type != expected_type) {
            fprintf(stderr, "Parsed token #%d expected type %s, gotten %s, code=\"%s\")\n", 
                        i, token_type_str(expected_type), token_type_str(actual_type), code);
            return false;
        }
        if (actual_type == T_IDENTIFIER || actual_type == T_NUMBER_LITERAL || actual_type == T_STRING_LITERAL || actual_type == T_BOOLEAN_LITERAL) {
            char *expected_data = va_arg(args, char *);
            const char *actual_data = token_get_data(t);
            if (strcmp(actual_data, expected_data) != 0) {
                fprintf(stderr, "Parsed token #%d expected data \"%s\", gotten \"%s\", code=\"%s\")\n", 
                            i, expected_data, actual_data, code);
                return false;
            }
        }
    }
    va_end(args);

    return true;
}

bool tokenizer_self_diagnostics() {
    bool all_passed = true;
    
    // for each token, pass in expected type. for identifiers and literals, pass in data.
    if (!use_case_passes(NULL, false, 0)) 
        all_passed = false;
    if (!use_case_passes(":", true, 0))
        all_passed = false;
    if (!use_case_passes("", false, 1, T_END))
        all_passed = false;
    if (!use_case_passes("index", false, 2, T_IDENTIFIER, "index", T_END))
        all_passed = false;
    if (!use_case_passes("123", false, 2, T_NUMBER_LITERAL, "123", T_END))
        all_passed = false;
    if (!use_case_passes("'hello'", false, 2, T_STRING_LITERAL, "hello", T_END))
        all_passed = false;
    if (!use_case_passes("\" abc \"", false, 2, T_STRING_LITERAL, " abc ", T_END))
        all_passed = false;
    if (!use_case_passes("true", false, 2, T_BOOLEAN_LITERAL, "true", T_END))
        all_passed = false;
    if (!use_case_passes("false", false, 2, T_BOOLEAN_LITERAL, "false", T_END))
        all_passed = false;
    if (!use_case_passes(">", false, 2, T_LARGER, T_END))
        all_passed = false;
    if (!use_case_passes(">=", false, 2, T_LARGER_EQUAL, T_END))
        all_passed = false;
    if (!use_case_passes(">>", false, 2, T_DOUBLE_LARGER, T_END))
        all_passed = false;
    if (!use_case_passes("if(a >= 10, b, c)", false, 11, 
        T_IDENTIFIER, "if",
        T_LPAREN,
        T_IDENTIFIER, "a",
        T_LARGER_EQUAL,
        T_NUMBER_LITERAL, "10",
        T_COMMA,
        T_IDENTIFIER, "b",
        T_COMMA,
        T_IDENTIFIER, "c",
        T_RPAREN,
        T_END))
        all_passed = false;

    return all_passed;
}
