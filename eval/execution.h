#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "../utils/value.h"
#include "../utils/dict.h"
#include "expression.h"


failable_value execute_expression(expression *e, dict *values);


#endif
