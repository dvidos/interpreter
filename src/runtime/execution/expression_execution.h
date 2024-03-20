#ifndef _EXPRESSION_EXECUTION_H
#define _EXPRESSION_EXECUTION_H

#include "../../containers/_containers.h"
#include "../../entities/_entities.h"


void initialize_expression_execution();

execution_outcome execute_expression(expression *e, exec_context *ctx);


#endif
