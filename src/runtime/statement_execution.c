#include <stdlib.h>
#include <string.h>
#include "../debugger/debugger.h"
#include "../utils/str_builder.h"
#include "../utils/data_types/_module.h"
#include "expression_execution.h"
#include "statement_execution.h"


static failable_bool check_condition(expression *condition, exec_context *ctx);
static failable_variant execute_single_statement(statement *stmt, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_once(list *statements, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_in_loop(expression *condition, list *statements, expression *next, exec_context *ctx, bool *should_return);

failable_variant statement_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx);


// public entry point
failable_variant execute_statements(list *statements, exec_context *ctx) {
    bool should_break = false;
    bool should_continue = false;
    bool should_return = false;
    failable_variant execution = execute_statements_once(statements, ctx, &should_break, &should_continue, &should_return);
    if (execution.failed) return failed_variant(&execution, NULL);
    return ok_variant(execution.result);
}

static failable_bool check_condition(expression *condition, exec_context *ctx) {
    failable_variant exec = execute_expression(condition, ctx);
    if (exec.failed) return failed_bool(&exec, NULL);
    if (!variant_is_bool(exec.result)) return failed_bool(NULL, "condition must result in a boolean value");
    return ok_bool(variant_as_bool(exec.result));
}

static failable_variant execute_single_statement(statement *stmt, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return) {
    statement_type s_type = stmt->type;
    failable_variant execution;
    variant *return_value = new_null_variant();

    // not all statement types should be checked for debugger
    if (s_type != ST_EXPRESSION && s_type != ST_FUNCTION) {
        if (should_start_debugger(stmt, NULL, ctx)) {
            failable session = run_debugger(stmt, NULL, ctx);
            if (session.failed) return failed_variant(&session, NULL);
        }
    }

    if (s_type == ST_IF) {
        failable_bool pass_check = check_condition(stmt->per_type.if_.condition, ctx);
        if (pass_check.failed) return failed_variant(&pass_check, NULL);
        if (pass_check.result) {
            execution = execute_statements_once(stmt->per_type.if_.body_statements, ctx, should_break, should_continue, should_return);
        } else if (stmt->per_type.if_.has_else) { 
            execution = execute_statements_once(stmt->per_type.if_.else_body_statements, ctx, should_break, should_continue, should_return);
        } else {
            execution = ok_variant(return_value); // nothing to execute.
        }
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;
        
    } else if (s_type == ST_WHILE) {
        execution = execute_statements_in_loop(
            stmt->per_type.while_.condition,
            stmt->per_type.while_.body_statements,
            NULL,
            ctx,
            should_return
        );
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_FOR_LOOP) {
        execution = execute_expression(stmt->per_type.for_.init, ctx);
        if (execution.failed)
            return failed_variant(&execution, NULL);
        execution = execute_statements_in_loop(
            stmt->per_type.for_.condition,
            stmt->per_type.for_.body_statements,
            stmt->per_type.for_.next,
            ctx,
            should_return
        );
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_EXPRESSION) {
        execution = execute_expression(stmt->per_type.expr.expr, ctx);
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_BREAK) {
        *should_break = true;
    } else if (s_type == ST_CONTINUE) {
        *should_continue = true;
    } else if (s_type == ST_RETURN) {
        if (stmt->per_type.return_.value != NULL) {
            execution = execute_expression(stmt->per_type.return_.value, ctx);
            if (execution.failed)
                return failed_variant(&execution, NULL);
            return_value = execution.result;
        } else {
            return_value = new_null_variant();
        }
        *should_return = true;
        return_value = execution.result;

    } else if (s_type == ST_FUNCTION) {
        // this is a statement function, hence a named function. Register to symbols
        const char *name = stmt->per_type.function.name;
        exec_context_register_symbol(ctx, name, new_callable_variant(new_callable(
            name,
            statement_function_callable_executor,
            stmt
        )));
    } else if (s_type == ST_BREAKPOINT) {
        // ignored in execution
    } else {
        str_builder *sb = new_str_builder();
        statement_describe(stmt, sb);
        return failed_variant(NULL, "was expecting if, while, for, expression, return, but got %s", str_builder_charptr(sb));
    }

    return ok_variant(return_value);
}

static failable_variant execute_statements_once(list *statements, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return) {
    variant *return_value = new_null_variant();

    for_list(statements, it, statement, stmt) {
        failable_variant execution = execute_single_statement(stmt, ctx, should_break, should_continue, should_return);
        if (execution.failed) 
            return failed_variant(&execution, NULL);
        return_value = execution.result;
        if (*should_break || *should_continue || *should_return)
            break;
    }

    return ok_variant(return_value);
}

static failable_variant execute_statements_in_loop(expression *pre_condition, list *statements, expression *next, exec_context *ctx, bool *should_return) {
    iterator *it = list_iterator(statements);
    variant *return_value = new_null_variant();
    failable_variant execution;

    while (true) {
        failable_bool cond_check = check_condition(pre_condition, ctx);
        if (cond_check.failed) return failed_variant(&cond_check, NULL);
        if (!cond_check.result) break;

        bool should_break = false;
        bool should_continue = false;
    
        for_iterator(it, statement, stmt) {
            execution = execute_single_statement(stmt, ctx, &should_break, &should_continue, should_return);
            if (execution.failed) return failed_variant(&execution, NULL);
            return_value = execution.result;
            if (should_break || should_continue || *should_return)
                break;
        }

        // "continue" in "for" statements allows the "next" operation to run
        if (should_break) break;
        if (*should_return) return ok_variant(return_value);

        if (next != NULL) {
            execution = execute_expression(next, ctx);
            if (execution.failed) return failed_variant(&execution, NULL);
        }
    }

    return ok_variant(return_value);
}

failable_variant statement_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx) {

    statement *stmt = (statement *)callable_data;
    list *arg_names = stmt->per_type.function.arg_names;
    if (list_length(positional_args) < list_length(arg_names))
        return failed_variant(NULL, "expected %d arguments, got %d", list_length(arg_names), list_length(positional_args));

    stack_frame *f = new_stack_frame(stmt->per_type.function.name, stmt, NULL);
    stack_frame_initialization(f, arg_names, positional_args, named_args, NULL);
    exec_context_push_stack_frame(ctx, f);
    
    failable_variant result = execute_statements(stmt->per_type.function.statements, ctx);

    exec_context_pop_stack_frame(ctx);
    return result;
}


