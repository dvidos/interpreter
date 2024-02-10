#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../utils/failable.h"
#include "tokenization.h"



// read these values in this module, use the code_*() functions for changes
static const char *code_curr_char = NULL;
static const char *code_filename = NULL;
static int code_line_no = 1;
static int code_column_no = 1;

static const void code_reset_pos(const char *code, const char *filename) {
    code_curr_char = code;
    code_filename = filename;
    code_line_no = 1;
    code_column_no = 1;
}
static const void code_advance_pos() {
    if (code_curr_char == NULL || *code_curr_char == '\0')
        return;
    
    if (*code_curr_char == '\n') {
        code_line_no += 1;
        code_column_no = 0;
    }
    
    code_column_no++;
    code_curr_char++;
}
static inline bool code_finished() {
    return *code_curr_char == '\0';
}

// ----------------------


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

static struct reserved_word {
    const char *word;
    token_type token;
} reserved_words[] = {
    { "if",         T_IF },
    { "else",       T_ELSE },
    { "while",      T_WHILE },
    { "for",        T_FOR },
    { "break",      T_BREAK },
    { "continue",   T_CONTINUE },
    { "return",     T_RETURN },
    { "function",   T_FUNCTION_KEYWORD },
    { "breakpoint", T_BREAKPOINT },
};

static token_type get_reserved_word_token(const char *data) {
    for (int i = 0; i < sizeof(reserved_words)/sizeof(reserved_words[0]); i++)
        if (strcmp(reserved_words[i].word, data) == 0)
            return reserved_words[i].token;
    return T_UNKNOWN;
}

static char *collect(char_filter_function *filter) {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    while (!code_finished()
        && (filter(*code_curr_char))
        && (strlen(buffer) < sizeof(buffer) - 1) 
    ) {
        buffer[strlen(buffer)] = *code_curr_char;
        code_advance_pos();
    }

    char *data = malloc(strlen(buffer) + 1);
    strcpy(data, buffer);
    return data;
}

static char *collect_string_literal() {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    char quote = *code_curr_char;
    code_advance_pos();
    
    while (!code_finished()
        && (*code_curr_char != quote)
        && (strlen(buffer) < sizeof(buffer) - 1) 
    ) {
        buffer[strlen(buffer)] = *code_curr_char;
        code_advance_pos();
    }

    // skip over closing quote
    if (*code_curr_char == quote)
        code_advance_pos();

    char *data = malloc(strlen(buffer) + 1);
    strcpy(data, buffer);
    return data;
}

static void skip_whitespace() {
    while (!code_finished() && is_whitespace(*code_curr_char))
        code_advance_pos();
}

static void skip_comment(bool is_block) {
    while (!code_finished()) {
        bool found_end = is_block ? 
            (*code_curr_char == '*' && *(code_curr_char + 1) == '/') :
            (*code_curr_char == '\n');
        if (found_end) {
            code_advance_pos();
            if (is_block) code_advance_pos();
            break;
        }
        code_advance_pos();
    }
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

void initialize_lexer() {
    tokens_trie_root = malloc(sizeof(tokens_trie_node));
    memset(tokens_trie_root, 0, sizeof(tokens_trie_node));
    for (int tt = 0; tt < T_MAX_VALUE; tt++) {
        const char *parse_chars = token_type_parse_chars(tt);
        if (parse_chars == NULL || strlen(parse_chars) == 0)
            continue;
        place_token_in_tokens_trie(parse_chars, tt);
    }
}

static token_type get_char_token_type() {
    // use the trie
    token_type result = T_UNKNOWN;
    tokens_trie_node *curr = tokens_trie_root;
    while (curr->children[*code_curr_char] != NULL) {
        // we may have something.
        curr = curr->children[*code_curr_char];
        result = curr->type;
        code_advance_pos();
    }

    return result;
}

static failable_token get_token_at_code_position() {

    skip_whitespace();
    if (code_finished())
        return ok_token(NULL);
    
    // try a char-based token first
    token_type char_token_type = get_char_token_type();
    if (char_token_type != T_UNKNOWN) {
        return ok_token(new_token(char_token_type, code_filename, code_line_no, code_column_no));
    }

    char c = *code_curr_char;
    if (c == '"' || c == '\'') {
        char *data = collect_string_literal();
        return ok_token(new_data_token(T_STRING_LITERAL, data, code_filename, code_line_no, code_column_no));

    } else if (is_number_char(c)) {
        char *data = collect(is_number_char);
        return ok_token(new_data_token(T_NUMBER_LITERAL, data, code_filename, code_line_no, code_column_no));

    } else if (is_identifier_char(c)) {
        char *data = collect(is_identifier_char);
        token_type reserved_word_token = get_reserved_word_token(data);
        if (reserved_word_token != T_UNKNOWN)
            return ok_token(new_token(reserved_word_token, code_filename, code_line_no, code_column_no));
        else if (strcmp(data, "true") == 0 || strcmp(data, "false") == 0)
            return ok_token(new_data_token(T_BOOLEAN_LITERAL, data, code_filename, code_line_no, code_column_no));
        else
            return ok_token(new_data_token(T_IDENTIFIER, data, code_filename, code_line_no, code_column_no));
    }
    
    return failed_token(NULL, "Unrecognized character '%c' at %s:%d:%d", *code_curr_char, code_filename, code_line_no, code_column_no);
}

failable_list parse_code_into_tokens(const char *code, const char *filename) {
    list *tokens = new_list(token_class);

    if (code == NULL || strlen(code) == 0) {
        list_add(tokens, new_token(T_END, code_filename, code_line_no, code_column_no));
        return ok_list(tokens);
    }
    
    code_reset_pos(code, filename);
    while (!code_finished()) {
        failable_token t = get_token_at_code_position();
        if (t.failed)
            return failed_list(&t, "Cannot get token");
        if (t.result == NULL)
            break;
        
        token_type tt = t.result->type;
        if (tt == T_DOUBLE_SLASH || tt == T_SLASH_STAR) {
            skip_comment(tt == T_SLASH_STAR);
            continue;
        }
        
        list_add(tokens, t.result);
    }
    list_add(tokens, new_token(T_END, code_filename, code_line_no, code_column_no));

    return ok_list(tokens);
}

