#ifndef _TOKENIZATION_H
#define _TOKENIZATION_H

#include "../../utils/containers/_module.h"
#include "../entities/_module.h"

void initialize_lexer();

failable_list parse_code_into_tokens(const char *code, const char *filename);

#endif
