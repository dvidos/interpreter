#ifndef _PARSER_MODULE_H
#define _PARSER_MODULE_H

// this file contains public interface of the whole module,
// discrete header files conain interface for the files within the module.

typedef enum completion_mode {
    CM_END_OF_TEXT,        // till end of tokens
    CM_RPAREN,             // a closing parenthesis
    CM_SEMICOLON,          // explicit semicolon
    CM_SEMICOLON_OR_END,   // for parsing simple expressions e.g. "(a+b)/2"
    CM_FUNC_ARGS,          // either comma or closing parenthesis
    CM_COLON               // until a colon is found, used for '?:'
} completion_mode;


void initialize_statement_parser();
void initialize_expression_parser();

failable_list parse_statements(iterator *tokens, bool parse_single_statement_if_no_block);
failable_statement parse_statement(iterator *tokens);
failable_expression parse_expression(iterator *tokens, completion_mode completion, bool verbose);

bool statement_parser_self_diagnostics(bool verbose);
bool expression_parser_self_diagnostics(bool verbose);



#endif
