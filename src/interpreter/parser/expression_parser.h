#ifndef _EXPRESSION_PARSER_H
#define _EXPRESSION_PARSER_H

#include <stdbool.h>
#include "../../utils/iterator.h"
#include "../entities/_module.h"

typedef enum completion_mode {
    CM_END_OF_TEXT,         // till end of tokens
    CM_RPAREN,              // a closing parenthesis
    CM_RSQBRACKET,          // a closing square bracket
    CM_SEMICOLON,           // explicit semicolon
    CM_SEMICOLON_OR_END,    // for parsing simple expressions e.g. "(a+b)/2"
    CM_COMMA_OR_RPAREN,     // either comma or closing parenthesis
    CM_COMMA_OR_RSQBRACKET, // either comma or closing square bracket
    CM_COMMA_OR_RBRACKET,   // either comma or closing bracket
    CM_COLON                // until a colon is found, used for '?:'
} completion_mode;


void initialize_expression_parser();

failable_expression parse_expression(iterator *tokens, completion_mode completion, bool verbose);


#endif
