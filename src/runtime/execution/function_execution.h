#ifndef _FUNCTION_EXECUTION_H
#define _FUNCTION_EXECUTION_H

#include "../../containers/_containers.h"
#include "../variants/_variants.h"


execution_outcome execute_user_function(
    const char *name,
    list *func_statements, 
    list *func_arg_names,
    list *arg_values, 
    variant *this_obj,
    origin *call_origin,
    exec_context *ctx);


#endif
