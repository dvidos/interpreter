#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../utils/failable.h"
#include "tokenization.h"
#include "token.h"


static bool is_whitespace(char c) {
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

typedef struct tokens_trie_node {
    struct tokens_trie_node *children[128];
    token_type type;
} tokens_trie_node;

static tokens_trie_node *tokens_trie_root = NULL;

static void place_token_in_tokens_trie(const char *parse_chars, token_type type) {
    tokens_trie_node *curr = tokens_trie_root;
    for (int i = 0; i < strlen(parse_chars); i++) {
        char c = parse_chars[i];
        if (curr->children[c] == NULL) {
            tokens_trie_node *child = malloc(sizeof(tokens_trie_node));
            memset(child, 0, sizeof(tokens_trie_node));
            curr->children[c] = child;
        }
        curr = curr->children[c];
    }
    curr->type = type;
}

void initialize_char_tokens_trie() {
    tokens_trie_root = malloc(sizeof(tokens_trie_node));
    memset(tokens_trie_root, 0, sizeof(tokens_trie_node));
    for (int tt = 0; tt < T_MAX_VALUE; tt++) {
        const char *parse_chars = token_type_parse_chars(tt);
        if (parse_chars == NULL || strlen(parse_chars) == 0)
            continue;
        place_token_in_tokens_trie(parse_chars, tt);
    }
}

static token_type get_char_token_type(const char *code, int len, int *pos) {
    // use the trie
    token_type result = T_UNKNOWN;
    tokens_trie_node *curr = tokens_trie_root;
    while (curr->children[code[*pos]] != NULL) {
        // we may have something.
        curr = curr->children[code[*pos]];
        result = curr->type;
        (*pos) += 1;
    }

    return result;
}

static failable_token get_token_at_code_position(const char *code, int len, int *pos) {

    // skip whitespacae
    while (*pos < len && is_whitespace(code[*pos]))
        (*pos) += 1;
    if (*pos >= len)
        return ok_token(NULL);
    
    // try a char-based token first
    token_type char_token_type = get_char_token_type(code, len, pos);
    if (char_token_type != T_UNKNOWN) {
        return ok_token(new_token(char_token_type));
    }

    char c = code[(*pos)++];
    if (c == '"' || c == '\'') {
        char *data = collect_string_literal(code, len, pos, c);
        return ok_token(new_data_token(T_STRING_LITERAL, data));

    } else if (is_number_char(c)) {
        char *data = collect(code, len, pos, c, is_number_char);
        return ok_token(new_data_token(T_NUMBER_LITERAL, data));

    } else if (is_identifier_char(c)) {
        char *data = collect(code, len, pos, c, is_identifier_char);
        if (strcmp(data, "true") == 0 || strcmp(data, "false") == 0)
            return ok_token(new_data_token(T_BOOLEAN_LITERAL, data));
        else
            return ok_token(new_data_token(T_IDENTIFIER, data));
    }
    
    return failed("Unrecognized token at pos %d ('%c')", (*pos)-1, code[(*pos)-1]);
}

failable_list parse_code_into_tokens(const char *code) {
    list *tokens = new_list(containing_tokens);

    if (code == NULL) {
        list_add(tokens, new_token(T_END));
        return ok_list(tokens);
    }
    
    int len = strlen(code);
    int pos = 0;
    while (pos < len) {
        failable_token t = get_token_at_code_position(code, len, &pos);
        if (t.failed)
            return failed("Cannot get token: %s", t.err_msg);
        if (t.result == NULL)
            break;
        list_add(tokens, t.result);
    }
    list_add(tokens, new_token(T_END));

    return ok_list(tokens);
}

