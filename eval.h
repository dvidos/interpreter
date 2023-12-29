#ifndef _EVAL_H
#define _EVAL_H

#include "value.h"
#include "dict.h"

// evaluate an expression, could be a function actually...
value *evaluate(const char *expression, dict *arguments);


#endif
