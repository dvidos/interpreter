#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "../utils/variant.h"
#include "../utils/containers/dict.h"

void initialize_interpreter();
failable_variant interpret_and_execute(const char *code, dict *arguments, bool verbose);

#endif
