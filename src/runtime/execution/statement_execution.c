#include <stdlib.h>
#include <string.h>
#include "../../debugger/debugger.h"
#include "../../utils/str_builder.h"
#include "../../utils/data_types/_module.h"
#include "expression_execution.h"
#include "statement_execution.h"


static execution_outcome check_condition(expression *condition, exec_context *ctx);
static execution_outcome execute_single_statement(statement *stmt, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static execution_outcome execute_statements_with_flow(list *statements, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static execution_outcome execute_statements_in_loop(expression *condition, list *statements, expression *next, exec_context *ctx, bool *should_return);

execution_outcome statement_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx);


// public entry point
execution_outcome execute_statements(list *statements, exec_context *ctx) {
    bool should_break = false;
    bool should_continue = false;
    bool should_return = false;
    return execute_statements_with_flow(statements, ctx, &should_break, &should_continue, &should_return);
}

static execution_outcome check_condition(expression *condition, exec_context *ctx) {
    execution_outcome ex = execute_expression(condition, ctx);
    if (ex.exception_thrown || ex.failed) return ex;
    if (!variant_instance_of(ex.result, bool_type))
        return exception_outcome(new_exception_variant(condition->token->filename, condition->token->line_no, condition->token->column_no, NULL,
            "condition expressions must yield boolean result"));
    
    return ok_outcome(ex.result);
}

static execution_outcome execute_single_statement(statement *stmt, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return) {
    statement_type s_type = stmt->type;
    execution_outcome ex;
    variant *return_value = void_instance;

    // not all statement types should be checked for debugger
    if (s_type != ST_EXPRESSION && s_type != ST_FUNCTION) {
        if (should_start_debugger(stmt, NULL, ctx)) {
            failable session = run_debugger(stmt, NULL, ctx);
            if (session.failed) return failed_outcome(session.err_msg);
        }
    }

    if (s_type == ST_IF) {
        ex = check_condition(stmt->per_type.if_.condition, ctx);
        if (ex.exception_thrown || ex.failed)
            return ex;
        bool passed = bool_variant_as_bool(ex.result);

        if (passed) {
            ex = execute_statements_with_flow(stmt->per_type.if_.body_statements, ctx, should_break, should_continue, should_return);
        } else if (stmt->per_type.if_.has_else) { 
            ex = execute_statements_with_flow(stmt->per_type.if_.else_body_statements, ctx, should_break, should_continue, should_return);
        } else {
            ex = ok_outcome(ex.result);
        }
        if (ex.exception_thrown || ex.failed)
            return ex;
        return_value = ex.result;
        
    } else if (s_type == ST_WHILE) {
        ex = execute_statements_in_loop(
            stmt->per_type.while_.condition,
            stmt->per_type.while_.body_statements,
            NULL,
            ctx,
            should_return
        );
        if (ex.exception_thrown || ex.failed)
            return ex;
        return_value = ex.result;

    } else if (s_type == ST_FOR_LOOP) {
        ex = execute_expression(stmt->per_type.for_.init, ctx);
        if (ex.exception_thrown || ex.failed)
            return ex;
        ex = execute_statements_in_loop(
            stmt->per_type.for_.condition,
            stmt->per_type.for_.body_statements,
            stmt->per_type.for_.next,
            ctx,
            should_return
        );
        if (ex.exception_thrown || ex.failed)
            return ex;
        return_value = ex.result;

    } else if (s_type == ST_EXPRESSION) {
        ex = execute_expression(stmt->per_type.expr.expr, ctx);
        if (ex.exception_thrown || ex.failed)
            return ex;
        return_value = ex.result;

    } else if (s_type == ST_BREAK) {
        if (should_break != NULL)
            *should_break = true;

    } else if (s_type == ST_CONTINUE) {
        if (should_continue != NULL)
            *should_continue = true;

    } else if (s_type == ST_RETURN) {
        if (stmt->per_type.return_.value != NULL) {
            ex = execute_expression(stmt->per_type.return_.value, ctx);
            if (ex.exception_thrown || ex.failed)
                return ex;
            return_value = ex.result;
        } else {
            return_value = void_instance;
        }
        if (should_return != NULL)
            *should_return = true;

    } else if (s_type == ST_FUNCTION) {
        // this is a statement function, hence a named one. Register to symbols
        exec_context_register_symbol(ctx, stmt->per_type.function.name,
            new_callable_variant(new_callable(
                stmt->per_type.function.name,
                statement_function_callable_executor,
                stmt
            ))
        );

    } else if (s_type == ST_TRY_CATCH) {
        ex = execute_statements_with_flow(stmt->per_type.try_catch.try_statements, ctx, should_break, should_continue, should_return);
        if (ex.failed) return ex;

        // save this for later, we may run a "finally" block
        execution_outcome try_catch_outcome = ex;

        if (ex.exception_thrown && stmt->per_type.try_catch.catch_statements != NULL) {
            variant *exception = ex.exception;

            if (stmt->per_type.try_catch.exception_identifier != NULL)
                exec_context_register_symbol(ctx, stmt->per_type.try_catch.exception_identifier, exception);
            ex = execute_statements_with_flow(stmt->per_type.try_catch.catch_statements, ctx, should_break, should_continue, should_return);
            if (ex.failed) return ex;
            if (stmt->per_type.try_catch.exception_identifier != NULL)
                exec_context_unregister_symbol(ctx, stmt->per_type.try_catch.exception_identifier);
            
            // if exception was successfully handled, we are ok
            // if new exception was raised inside catch, save for post-finally
            try_catch_outcome = ex;
        }

        // finally will run in any case, but will not influence the result.
        if (stmt->per_type.try_catch.finally_statements != NULL) {
            ex = execute_statements_with_flow(stmt->per_type.try_catch.finally_statements, ctx, should_break, should_continue, should_return);    
            if (ex.exception_thrown || ex.failed) return ex;
        }
        
        return try_catch_outcome;

    } else if (s_type == ST_THROW) {
        // calculate thrown value, just a string for now
        const char *msg = "";
        if (stmt->per_type.throw.exception != NULL) {
            ex = execute_expression(stmt->per_type.throw.exception, ctx);
            if (ex.exception_thrown || ex.failed) return ex;
            msg = deprecated_variant_as_const_char(ex.result);
        }
        return exception_outcome(new_exception_variant(
            stmt->token->filename, stmt->token->line_no, stmt->token->column_no, NULL, msg));
        
    } else if (s_type == ST_BREAKPOINT) {
        // ignored in execution, bebugger entry is checked before executing a row.
    } else {
        str_builder *sb = new_str_builder();
        statement_describe(stmt, sb);
        return exception_outcome(new_exception_variant(
            stmt->token->filename, stmt->token->line_no, stmt->token->column_no, NULL,
            "was expecting [ if, while, for, break, continue, expression, try, return, breakpoint ] but got %s", 
            str_builder_charptr(sb)
        ));
    }

    return ok_outcome(return_value);
}

static execution_outcome execute_statements_with_flow(list *statements, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return) {
    variant *return_value = void_instance;

    for_list(statements, it, statement, stmt) {
        execution_outcome ex = execute_single_statement(stmt, ctx, should_break, should_continue, should_return);
        if (ex.exception_thrown || ex.failed) return ex;
        return_value = ex.result;
        if (*should_break || *should_continue || *should_return)
            break;
    }

    return ok_outcome(return_value);
}

static execution_outcome execute_statements_in_loop(expression *pre_condition, list *statements, expression *next, exec_context *ctx, bool *should_return) {
    variant *return_value = void_instance;
    execution_outcome ex;

    while (true) {
        ex = check_condition(pre_condition, ctx);
        if (ex.exception_thrown || ex.failed) return ex;
        if (!bool_variant_as_bool(ex.result))
            break;

        bool should_break = false;
        bool should_continue = false;
    
        ex = execute_statements_with_flow(statements, ctx, &should_break, &should_continue, should_return);
        if (ex.exception_thrown || ex.failed) return ex;

        // "continue" in "for" statements allows the "next" operation to run
        if (should_break) break;
        if (*should_return) return ok_outcome(return_value);

        if (next != NULL) {
            ex = execute_expression(next, ctx);
            if (ex.exception_thrown || ex.failed) return ex;
        }
    }

    return ok_outcome(return_value);
}

execution_outcome statement_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx) {

    statement *stmt = (statement *)callable_data;
    list *arg_names = stmt->per_type.function.arg_names;
    if (list_length(positional_args) < list_length(arg_names)) {
        // we should report where the call was made, not where the function is
        return exception_outcome(new_exception_variant(
            stmt->token->filename, stmt->token->line_no, stmt->token->column_no, NULL,
            "%s() expected %d arguments, got %d", stmt->per_type.function.name, list_length(arg_names), list_length(positional_args)
        ));
    }

    stack_frame *frame = new_stack_frame(stmt->per_type.function.name, stmt, NULL);
    stack_frame_initialization(frame, arg_names, positional_args, named_args, NULL);
    exec_context_push_stack_frame(ctx, frame);
    
    execution_outcome result = execute_statements(stmt->per_type.function.statements, ctx);

    // even if an exception was raised, we still must pop the stack frame.
    exec_context_pop_stack_frame(ctx);
    return result;
}


