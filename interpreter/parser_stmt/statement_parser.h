#ifndef _STATEMENT_PARSER_H
#define _STATEMENT_PARSER_H

#include "../../utils/iterator.h"
#include "../../utils/containers/_module.h"
#include "statement.h"


void initialize_statement_parser();

failable_statement parse_statement(iterator *tokens);
failable_list parse_statements(iterator *tokens, bool parse_single_statement_if_no_block);


#endif
