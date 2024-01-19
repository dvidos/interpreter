#ifndef _RUNTIME_MODULE_H
#define _RUNTIME_MODULE_H


#include "../../utils/containers/_module.h"


failable_variant execute_statements(list *statements, dict *values, dict *callables);



#include "exec_context.h"
#include "built_in_funcs.h"


#endif
