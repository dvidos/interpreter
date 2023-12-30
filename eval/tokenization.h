#ifndef _TOKENIZATION_H
#define _TOKENIZATION_H

#include "../utils/status.h"
#include "../utils/list.h"

status parse_code_into_tokens(const char *code, list **tokens_out);


#endif
