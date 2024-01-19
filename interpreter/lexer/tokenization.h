#ifndef _TOKENIZATION_H
#define _TOKENIZATION_H

#include "../../utils/containers/_module.h"

void initialize_char_tokens_trie();

failable_list parse_code_into_tokens(const char *code);

#endif
