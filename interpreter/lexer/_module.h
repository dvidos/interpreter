#ifndef _LEXER_MODULE_H
#define _LEXER_MODULE_H

// this file contains public interface of the whole module,
// discrete header files conain interface for the files within the module.

#include <stdbool.h>
#include "../../utils/containers/_module.h"

// for main / higher level interface
void initialize_lexer();
bool lexer_self_diagnostics(bool verbose);
failable_list parse_code_into_tokens(const char *code, const char *filename);




#endif
