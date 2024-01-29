#include <stdlib.h>
#include <string.h>
#include "../utils/str_builder.h"
#include "../utils/data_types/_module.h"
#include "expression_execution.h"
#include "statement_execution.h"
#include "exec_context.h"

static failable_bool check_condition(expression *condition, exec_context *ctx);
static failable_variant execute_single_statement(statement *stmt, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_once(list *statements, exec_context *ctx, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_in_loop(expression *condition, list *statements, expression *next, exec_context *ctx, bool *should_return);

failable_variant statement_function_callable_executor(list *positional_args, dict *named_args, statement *stmt, exec_context *ctx);


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
    statement_type s_type = statement_get_type(stmt);
    failable_variant execution;
    variant *return_value = new_null_variant();

    if (s_type == ST_IF) {
        expression *condition = statement_get_expression(stmt, 0);
        failable_bool pass_check = check_condition(condition, ctx);
        if (pass_check.failed) return failed_variant(&pass_check, NULL);
        if (pass_check.result) {
            execution = execute_statements_once(statement_get_statements_body(stmt, false), ctx, should_break, should_continue, should_return);
        } else if (statement_has_alternate_body(stmt)) { 
            execution = execute_statements_once(statement_get_statements_body(stmt, true), ctx, should_break, should_continue, should_return);
        } else {
            execution = ok_variant(return_value); // nothing to execute.
        }
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;
        
    } else if (s_type == ST_WHILE) {
        execution = execute_statements_in_loop(
            statement_get_expression(stmt, 0),
            statement_get_statements_body(stmt, false),
            NULL,
            ctx,
            should_return
        );
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_FOR_LOOP) {
        execution = execute_expression(statement_get_expression(stmt, 0), ctx);
        if (execution.failed)
            return failed_variant(&execution, NULL);
        execution = execute_statements_in_loop(
            statement_get_expression(stmt, 1),
            statement_get_statements_body(stmt, false),
            statement_get_expression(stmt, 2),
            ctx,
            should_return
        );
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_EXPRESSION) {
        execution = execute_expression(statement_get_expression(stmt, 0), ctx);
        if (execution.failed)
            return failed_variant(&execution, NULL);
        return_value = execution.result;

    } else if (s_type == ST_BREAK) {
        *should_break = true;
    } else if (s_type == ST_CONTINUE) {
        *should_continue = true;
    } else if (s_type == ST_RETURN) {
        expression *ret_val_expr = statement_get_expression(stmt, 0);
        if (ret_val_expr != NULL) {
            execution = execute_expression(ret_val_expr, ctx);
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
        const char *name = statement_get_function_name(stmt);
        register_symbol(ctx->symbols, name, new_callable_variant(new_callable(
            name,
            (callable_handler *)statement_function_callable_executor,
            stmt
        )));
    } else {
        return failed_variant(NULL, "was expecting if, while, for, expression, return, but got %s", statement_to_string(stmt));
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


failable_variant statement_function_callable_executor(list *positional_args, dict *named_args, statement *stmt, exec_context *ctx) {

    list *arg_names = statement_get_function_arg_names(stmt);
    if (list_length(positional_args) != list_length(arg_names))
        return failed_variant(NULL, "expected %d arguments, got %d", list_length(arg_names), list_length(positional_args));

    symbol_table *local_symbols = new_symbol_table(ctx->symbols);
    if (positional_args != NULL) {
        for (int i = 0; i < list_length(positional_args); i++)
            register_symbol(local_symbols, list_get(arg_names, i), list_get(positional_args, i));
    }
    if (named_args != NULL) {
        for_dict(named_args, keys, str, key)
            register_symbol(local_symbols, key, dict_get(named_args, key));
    }
    ctx->symbols = local_symbols;
        
    failable_variant result = execute_statements(statement_get_statements_body(stmt, false), ctx);

    ctx->symbols = local_symbols->parent;
    return result;
}


