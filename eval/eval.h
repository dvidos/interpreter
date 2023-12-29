#ifndef _EVAL_H
#define _EVAL_H

#include "../utils/value.h"
#include "../utils/dict.h"

// evaluate an expression, could be a function actually...
value *evaluate(const char *expression, dict *arguments);


#endif
