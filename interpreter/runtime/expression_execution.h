#ifndef _EXPRESSION_EXECUTION_H
#define _EXPRESSION_EXECUTION_H

#include "../../utils/variant.h"
#include "../../utils/containers/_module.h"
#include "../parser_expr/expression.h"


failable_variant execute_expression(expression *e, dict *values, dict *callables);


#endif
