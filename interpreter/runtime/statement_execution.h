#ifndef _STATEMENT_EXECUTION_H
#define _STATEMENT_EXECUTION_H

#include "../../utils/variant.h"
#include "../../utils/containers/_module.h"
#include "../parser_stmt/statement.h"


failable_variant execute_statements(list *statements, dict *values, dict *callables);


#endif
