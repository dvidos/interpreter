#ifndef _STATEMENT_PARSER_H
#define _STATEMENT_PARSER_H

#include "../utils/iterator.h"
#include "../utils/containers/_module.h"
#include "../entities/_module.h"


typedef enum statement_parsing_mode statement_parsing_mode;
enum statement_parsing_mode {
    SP_SINGLE_OR_BLOCK,        // for example after an "if"
    SP_BLOCK_MANDATORY,        // for example a function's body
    SP_SEQUENTIAL_STATEMENTS,  // for example in a script.
};

void initialize_statement_parser();
failable_statement parse_statement(iterator *tokens);
failable_list parse_statements(iterator *tokens, statement_parsing_mode mode);


#endif
