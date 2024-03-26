
#include "function_execution.h"
#include "expression_execution.h"
#include "statement_execution.h"
#include "exec_context.h"
#include "stack_frame.h"

execution_outcome execute_user_function(
    const char *name,
    list *func_statements, 
    list *func_arg_names,
    list *arg_values, 
    variant *this_obj,
    origin *call_origin,
    exec_context *ctx) {
    
    // here we have opportunity for improvements
    // named arguments, or default values, variadic args etc.

    if (list_length(arg_values) < list_length(func_arg_names)) {
        return exception_outcome(new_exception_variant_at(
            call_origin, NULL, 
            "%s() expected %d arguments, got %d", name, list_length(func_arg_names), list_length(arg_values)
        ));
    }

    stack_frame *frame = new_stack_frame(name, call_origin);
    stack_frame_initialization(frame, func_arg_names, arg_values, this_obj, NULL);
    exec_context_push_stack_frame(ctx, frame);

    execution_outcome result = execute_statements(func_statements, ctx);
    
    // we could run local statistics or clean up local variables.
    
    exec_context_pop_stack_frame(ctx);

    return result;
}

