#ifndef _EVAL_H
#define _EVAL_H

#include "../utils/variant.h"
#include "../utils/containers/dict.h"

void initialize_evaluator();
failable_variant evaluate(const char *code, dict *arguments);
bool evaluator_self_diagnostics();


#endif
