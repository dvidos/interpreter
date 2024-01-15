#ifndef _EXPRESSION_PARSER_H
#define _EXPRESSION_PARSER_H

#include <stdbool.h>
#include "../../utils/iterator.h"
#include "expression.h"

typedef enum completion_mode {
    CM_NORMAL,         // either a semicolon, or end of tokens
    CM_RPAREN,         // a closing parenthesis
    CM_SEMICOLON,      // explicit semicolon
    CM_FUNC_ARGS,      // either comma or closing parenthesis
    CM_COLON           // until a colon is found, used for '?:'
} completion_mode;


void initialize_expression_parser();

failable_expression parse_expression(iterator *tokens, completion_mode completion, bool verbose);


#endif
