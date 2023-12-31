#ifndef _PARSER_H
#define _PARSER_H

#include "../utils/list.h"

failable_list parse_tokens_into_expressions(list *tokens);
bool parser_self_diagnostics();

#endif
