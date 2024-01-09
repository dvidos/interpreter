#ifndef _TOKENIZATION_H
#define _TOKENIZATION_H

#include "../utils/failable.h"
#include "../utils/containers/list.h"

void initialize_char_tokens_trie();
failable_list parse_code_into_tokens(const char *code);
bool tokenizer_self_diagnostics();

#endif
