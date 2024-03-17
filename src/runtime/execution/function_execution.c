
#include "function_execution.h"
#include "../"

execution_outcome execute_user_function(
    const char *name,
    list *func_statements, 
    list *arg_names,
    list *arg_expressions, 
    variant *this_obj,
    exec_context *ctx,
    char *call_filename,
    int call_line_no,
    int call_column_no) {
    
    // it can be either a statement function or an expression function
    // or a class method
    // prepare stack frame
    // set arguments as local variables
    // push stack frame
    // execute
    // pop stack frame
    // cleanup etc.

    if (list_length(positional_args) < list_length(declared_args)) {
        // we should report where the call was made, not where the function is
        return exception_outcome(new_exception_variant_at(
            expr->token->filename, expr->token->line_no, expr->token->column_no, NULL,
            "%s() expected %d arguments, got %d", expr->per_type.func.name, list_length(arg_names), list_length(positional_args)
        ));
    }

    stack_frame *frame = new_stack_frame(expr->per_type.func.name, NULL, expr);
    stack_frame_initialization(frame, arg_names, positional_args, this_obj);
    exec_context_push_stack_frame(ctx, frame);

    execution_outcome result = execute_statements(expr->per_type.func.statements, ctx);
    
    // we could run local statistics or clean up local variables.

    // even if an exception was raised, we still must pop the stack frame
    exec_context_pop_stack_frame(ctx);

    return result;
    
}

