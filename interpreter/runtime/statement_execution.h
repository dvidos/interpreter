#ifndef _STATEMENT_EXECUTION_H
#define _STATEMENT_EXECUTION_H

#include "../../utils/variant.h"
#include "../../utils/containers/dict.h"
#include "../../utils/containers/list.h"
#include "../parser_stmt/statement.h"


failable_variant execute_statements(list *statements, dict *values, dict *callables);


#endif
