#ifndef _EVAL_H
#define _EVAL_H

#include "../utils/value.h"
#include "../utils/dict.h"

failable_value evaluate(const char *code, dict *arguments);


#endif
