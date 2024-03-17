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
    exec_context *ctx,
    char *call_filename,
    int call_line_no,
    int call_column_no);


#endif
