#ifndef _TOKENIZATION_TESTS_H
#define _TOKENIZATION_TESTS_H

#include <stdbool.h>

void initialize_lexer();

bool lexer_self_diagnostics(bool verbose);

failable_list parse_code_into_tokens(const char *code, const char *filename);

#endif
