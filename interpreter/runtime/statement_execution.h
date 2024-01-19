#ifndef _STATEMENT_EXECUTION_H
#define _STATEMENT_EXECUTION_H

#include "../../utils/data_types/_module.h"
#include "../../utils/containers/_module.h"
#include "../entities/_module.h"


failable_variant execute_statements(list *statements, dict *values, dict *callables);


#endif
