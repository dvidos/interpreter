#ifndef _STATEMENT_EXECUTION_H
#define _STATEMENT_EXECUTION_H

#include "../../utils/data_types/_module.h"
#include "../../containers/_module.h"
#include "../../entities/_module.h"
#include "execution_outcome.h"

execution_outcome execute_statements(list *statements, exec_context *ctx);


#endif
