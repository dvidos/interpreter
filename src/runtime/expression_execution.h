#ifndef _EXPRESSION_EXECUTION_H
#define _EXPRESSION_EXECUTION_H

#include "../utils/data_types/_module.h"
#include "../utils/containers/_module.h"
#include "../entities/_module.h"
#include "exec_context.h"


void initialize_expression_execution();

failable_variant execute_expression(expression *e, exec_context *ctx);


#endif
