#ifndef _TOKEN_TYPE_H
#define _TOKEN_TYPE_H

typedef enum token_type {
    T_UNKNOWN = 0, // important that zero has unknown meaning

    T_IDENTIFIER,
    T_NUMBER_LITERAL,
    T_STRING_LITERAL,
    T_BOOLEAN_LITERAL,

    T_LPAREN,
    T_RPAREN,
    T_LBRACKET,    // '{'
    T_RBRACKET,    // '}'
    T_LSQBRACKET,  // '['
    T_RSQBRACKET,  // ']'
    T_SEMICOLON,
    T_PLUS,
    T_MINUS,
    T_ASTERISK,
    T_FWD_SLASH,
    T_DOUBLE_SLASH,
    T_SLASH_STAR,
    T_STAR_SLASH,
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
    T_DOUBLE_PLUS,
    T_DOUBLE_MINUS,
    T_PERCENT,
    T_CARET,
    T_QUESTION_MARK,
    T_COLON,
    T_PLUS_EQUAL,
    T_MINUS_EQUAL,
    T_STAR_EQUAL,
    T_SLASH_EQUAL,
    T_PERCENT_EQUAL,
    T_DBL_LARGER_EQUAL,
    T_DBL_SMALLER_EQUAL,
    T_AMPERSAND_EQUAL,
    T_PIPE_EQUAL,
    T_CARET_EQUAL,

    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    T_BREAK,
    T_CONTINUE,
    T_RETURN,
    T_FUNCTION_KEYWORD,
    T_TRY,
    T_CATCH,
    T_FINALLY,
    T_THROW,
    T_BREAKPOINT,
    T_CLASS,
    T_PUBLIC,

    T_END, // the last token of the stream
    T_MAX_VALUE // max value for the sizing of the enum
} token_type;


const char *token_type_str(token_type type);
const char *token_type_parse_chars(token_type type);


#endif
