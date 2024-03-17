#include "function_execution.h"


execution_outcome execute_function(
    list *func_statements, 
    list *args_declaration,
    list *arg_expressions, 
    variant *this_obj,
    exec_context *ctx) {
    
    // we should be given the token where the call is placed.
    // it can be either a statement function or an expression function


    // prepare stack frame
    // set arguments as local variables
    // push stack frame
    // execute
    // pop stack frame
    // cleanup etc.

    expression *expr = (expression *)callable_data;

    list *arg_names = expr->per_type.func.arg_names;
    if (list_length(positional_args) < list_length(arg_names)) {
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

