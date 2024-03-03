#ifndef _EXPRESSION_EXECUTION_H
#define _EXPRESSION_EXECUTION_H

#include "../../utils/data_types/_module.h"
#include "../../utils/containers/_module.h"
#include "../../entities/_module.h"
#include "execution_outcome.h"


void initialize_expression_execution();

execution_outcome execute_expression(expression *e, exec_context *ctx);


#endif
