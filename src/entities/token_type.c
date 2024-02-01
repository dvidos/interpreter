#include <stdlib.h>
#include <string.h>
#include "token_type.h"

struct token_type_info {
    token_type type;
    const char *parse_chars;
    const char *debug_str;
};

static struct token_type_info token_type_infos[] = {
    { T_UNKNOWN,            "",    "UNKNOWN" },
    { T_IDENTIFIER,         "",    "IDENTIFIER" },
    { T_NUMBER_LITERAL,     "",    "NUMBER_LITERAL" },
    { T_STRING_LITERAL,     "",    "STRING_LITERAL" },
    { T_BOOLEAN_LITERAL,    "",    "BOOLEAN_LITERAL" },

    { T_LPAREN,             "(",   "LPAREN" },
    { T_RPAREN,             ")",   "RPAREN" },
    { T_LBRACKET,           "{",   "LBRACKET" },
    { T_RBRACKET,           "}",   "RBRACKET" },
    { T_LSQBRACKET,         "[",   "LSQBRACKET" },
    { T_RSQBRACKET,         "]",   "RSQBRACKET" },
    { T_SEMICOLON,          ";",   "SEMICOLON" },
    { T_PLUS,               "+",   "PLUS" },
    { T_MINUS,              "-",   "MINUS" },
    { T_ASTERISK,           "*",   "ASTERISK" },
    { T_FWD_SLASH,          "/",   "FWD_SLASH" },
    { T_DOUBLE_SLASH,       "//",  "DOUBLE_SLASH" },
    { T_SLASH_STAR,         "/*",  "SLASH_STAR" },
    { T_STAR_SLASH,         "*/",  "STAR_SLASH" },
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
    { T_QUESTION_MARK,      "?",   "QUESTION_MARK" },
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
    
    { T_IF,                 "", "IF" },
    { T_ELSE,               "", "ELSE" },
    { T_WHILE,              "", "WHILE" },
    { T_FOR,                "", "FOR" },
    { T_BREAK,              "", "BREAK" },
    { T_CONTINUE,           "", "CONTINUE" },
    { T_RETURN,             "", "RETURN" },
    { T_FUNCTION_KEYWORD,   "", "FUNCTION" },

    { T_END,                "",    "END" },
    { T_MAX_VALUE,          "",    "MAX_VALUE" },
};


const char *token_type_str(token_type type) {
    for (int i = 0; i < sizeof(token_type_infos)/sizeof(token_type_infos[0]); i++) {
        if (token_type_infos[i].type == type)
            return token_type_infos[i].debug_str;
    }
    return "(unknown)";
}

const char *token_type_parse_chars(token_type type) {
    for (int i = 0; i < sizeof(token_type_infos)/sizeof(token_type_infos[0]); i++) {
        if (token_type_infos[i].type == type)
            return token_type_infos[i].parse_chars;
    }
    return "";
}

