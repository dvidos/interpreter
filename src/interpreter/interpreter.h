#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "../runtime/_runtime.h"

void initialize_interpreter();
execution_outcome interpret_and_execute(const char *code, const char *filename, dict *external_values, bool verbose, bool enable_debugger, bool start_with_debugger);

#endif
