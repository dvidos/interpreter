#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "../utils/data_types/_module.h"
#include "../containers/_module.h"
#include "../runtime/execution/execution_outcome.h"

void initialize_interpreter();
execution_outcome interpret_and_execute(const char *code, const char *filename, dict *external_values, bool verbose, bool enable_debugger, bool start_with_debugger);

#endif
