#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "../utils/data_types/_module.h"
#include "../utils/containers/_module.h"

void initialize_interpreter();
failable_variant interpret_and_execute(const char *code, const char *filename, dict *external_values, bool verbose, bool debugger);

#endif
