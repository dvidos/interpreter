#ifndef _EVAL_H
#define _EVAL_H

#include "../utils/value.h"
#include "../utils/dict.h"

status evaluate(const char *code, dict *arguments, value **result_out);


#endif
