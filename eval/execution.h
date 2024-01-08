#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "../utils/variant.h"
#include "../utils/dict.h"
#include "expression.h"


failable_variant execute_expression(expression *e, dict *values);


#endif
