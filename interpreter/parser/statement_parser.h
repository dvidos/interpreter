#ifndef _STATEMENT_PARSER_H
#define _STATEMENT_PARSER_H

#include <stdbool.h>
#include "../../utils/iterator.h"
#include "statement.h"


void initialize_statement_parser();

failable_statement parse_statement(iterator *tokens, bool verbose);


#endif
