#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "../../utils/variant.h"
#include "../../utils/containers/dict.h"
#include "../parser_expr/expression.h"


failable_variant execute_expression(expression *e, dict *values, dict *callables);


#endif
