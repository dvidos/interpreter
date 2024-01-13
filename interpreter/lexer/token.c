#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "../../utils/containers/containable.h"
#include "../../utils/strbld.h"

struct token_info {
    token_type type;
    const char *parse_chars;
    const char *debug_str;
};

contained_item_info *containing_tokens = &(contained_item_info){
    .are_equal = (are_equal_func)tokens_are_equal,
    .to_string = (to_string_func)token_to_string,
    .hash = NULL
};


static struct token_info token_infos[] = {
    { T_UNKNOWN,            "",    "UNKNOWN" },
    { T_IDENTIFIER,         "",    "IDENTIFIER" },
    { T_NUMBER_LITERAL,     "",    "NUMBER_LITERAL" },
    { T_STRING_LITERAL,     "",    "STRING_LITERAL" },
    { T_BOOLEAN_LITERAL,    "",    "BOOLEAN_LITERAL" },

    { T_LPAREN,             "(",   "LPAREN" },
    { T_RPAREN,             ")",   "RPAREN" },
    { T_LSQBRACKET,         "[",   "LSQBRACKET" },
    { T_RSQBRACKET,         "]",   "RSQBRACKET" },
    { T_SEMICOLON,          ";",   "SEMICOLON" },
    { T_PLUS,               "+",   "PLUS" },
    { T_MINUS,              "-",   "MINUS" },
    { T_ASTERISK,           "*",   "ASTERISK" },
    { T_FWD_SLASH,          "/",   "FWD_SLASH" },
    { T_DOUBLE_SLASH,       "//",  "DOUBLE_SLASH" },
    { T_PIPE,               "|",   "PIPE" },
    { T_DOUBLE_PIPE,        "||",  "DOUBLE_PIPE" },
    { T_AMPERSAND,          "&",   "AMPERSAND" },
    { T_DOUBLE_AMPERSAND,   "&&",  "DOUBLE_AMPERSAND" },
    { T_TIDLE,              "~",   "TIDLE" },
    { T_EXCLAMATION,        "!",   "EXCLAMATION" },
    { T_EXCLAMATION_EQUAL,  "!=",  "EXCLAMATION_EQUAL" },
    { T_EQUAL,              "=",   "EQUAL" },
    { T_DOUBLE_EQUAL,       "==",  "DOUBLE_EQUAL" },
    { T_LARGER_EQUAL,       ">=",  "LARGER_EQUAL" },
    { T_LARGER,             ">",   "LARGER" },
    { T_DOUBLE_LARGER,      ">>",  "DOUBLE_LARGER" },
    { T_SMALLER_EQUAL,      "<=",  "SMALLER_EQUAL" },
    { T_SMALLER,            "<",   "SMALLER" },
    { T_DOUBLE_SMALLER,     "<<",  "DOUBLE_SMALLER" },
    { T_ARROW,              "->",  "ARROW" },
    { T_DOT,                ".",   "DOT" },
    { T_COMMA,              ",",   "COMMA" },
    { T_DOUBLE_PLUS,        "++",  "DOUBLE_PLUS" },
    { T_DOUBLE_MINUS,       "--",  "DOUBLE_MINUS" },
    { T_PERCENT,            "%",   "PERCENT" },
    { T_CARET,              "^",   "CARET" },
    { T_QUESTION,           "?",   "QUESTION" },
    { T_COLON,              ":",   "COLON" },
    { T_PLUS_EQUAL,         "+=",  "PLUS_EQUAL" },
    { T_MINUS_EQUAL,        "-=",  "MINUS_EQUAL" },
    { T_STAR_EQUAL,         "*=",  "STAR_EQUAL" },
    { T_SLASH_EQUAL,        "/=",  "SLASH_EQUAL" },
    { T_PERCENT_EQUAL,      "%=",  "PERCENT_EQUAL" },
    { T_DBL_LARGER_EQUAL,   ">>=", "DBL_LARGER_EQUAL" },
    { T_DBL_SMALLER_EQUAL,  "<<=", "DBL_SMALLER_EQUAL" },
    { T_AMPERSAND_EQUAL,    "&=",  "AMPERSAND_EQUAL" },
    { T_PIPE_EQUAL,         "|=",  "PIPE_EQUAL" },
    { T_CARET_EQUAL,        "^=",  "CARET_EQUAL" },
    
    { T_END,                "",    "END" },
    { T_MAX_VALUE,          "",    "MAX_VALUE" },
};

struct token {
    containable *c;
    token_type type;
    const char *data; // e.g. identifier or number
};

token *new_token(token_type type) {
    token *t = malloc(sizeof(token));
    t->c = new_containable("token", 
        (are_equal_func)tokens_are_equal,
        (to_string_func)token_to_string
    );
    t->type = type;
    t->data = NULL;
    return t;
}

token *new_data_token(token_type type, const char *data) {
    token *t = malloc(sizeof(token));
    t->c = new_containable("token", 
        (are_equal_func)tokens_are_equal,
        (to_string_func)token_to_string
    );
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
    for (int i = 0; i < sizeof(token_infos)/sizeof(token_infos[0]); i++) {
        if (token_infos[i].type == type)
            return token_infos[i].debug_str;
    }
    return "(unknown)";
}

const char *token_type_parse_chars(token_type type) {
    for (int i = 0; i < sizeof(token_infos)/sizeof(token_infos[0]); i++) {
        if (token_infos[i].type == type)
            return token_infos[i].parse_chars;
    }
    return "";
}

void token_print(token *t, FILE *stream, char *prefix) {
    token_type tt = token_get_type(t);
    const char *data = token_get_data(t);
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);

    fprintf(stream, "%s%s%s%s%s", 
        prefix,
        token_type_str(tt), 
        has_data ? " \"" : "",
        has_data ? data : "",
        has_data ? "\"" : ""
    );
}

const char *token_to_string(token *t) {
    strbld *sb = new_strbld();

    token_type tt = token_get_type(t);
    const char *data = token_get_data(t);
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);
    
    strbld_cat(sb, token_type_str(tt));
    if (has_data)
        strbld_catf(sb, "(\"%s\")", t->data);
    
    return strbld_charptr(sb);
}

void token_print_list(list *tokens, FILE *stream, char *prefix, char *separator) {
    int i = 0;
    for_list(tokens, it, token, t) {
        if (i++ > 0)
            fprintf(stream, "%s", separator);
        token_print(t, stream, prefix);
    }
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
