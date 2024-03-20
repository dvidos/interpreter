#ifndef _TOKENIZATION_H
#define _TOKENIZATION_H

#include "../containers/_containers.h"
#include "../entities/_entities.h"

void initialize_lexer();

failable_list parse_code_into_tokens(const char *code, const char *filename);

#endif
