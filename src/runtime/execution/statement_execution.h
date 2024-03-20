#ifndef _STATEMENT_EXECUTION_H
#define _STATEMENT_EXECUTION_H

#include "../variants/_variants.h"
#include "../../containers/_containers.h"
#include "../../entities/_entities.h"

execution_outcome execute_statements(list *statements, exec_context *ctx);


#endif
