#ifndef _FUNCTION_EXECUTION_H
#define _FUNCTION_EXECUTION_H

#include "../../containers/_module.h"
#include "../variants/_module.h"


execution_outcome execute_user_function(
    const char *name,
    list *func_statements, 
    list *func_arg_names,
    list *arg_expressions, 
    variant *this_obj,
    origin *call_origin,
    exec_context *ctx);


#endif
