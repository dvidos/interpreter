#ifndef _STATEMENT_PARSER_H
#define _STATEMENT_PARSER_H

#include "../../utils/iterator.h"
#include "../../utils/containers/list.h"
#include "statement.h"


void initialize_statement_parser();

failable_statement parse_statement(iterator *tokens);
failable_list parse_statements_block(iterator *tokens);


#endif
