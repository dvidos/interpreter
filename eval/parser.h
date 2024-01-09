#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include "../utils/containers/list.h"

failable_list parse_tokens_into_expressions(list *tokens, bool verbose);
bool parser_self_diagnostics(bool verbose);

#endif
