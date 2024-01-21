#include <stdlib.h>
#include <string.h>
#include "../../utils/str_builder.h"
#include "../../utils/data_types/_module.h"
#include "expression_execution.h"
#include "statement_execution.h"

static dict *the_values;
static dict *the_callables;

static failable_bool check_condition(expression *condition);
static failable_variant execute_single_statement(statement *stmt, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_once(list *statements, bool *should_break, bool *should_continue, bool *should_return);
static failable_variant execute_statements_in_loop(expression *condition, list *statements, expression *next, bool *should_return);


// public entry point
failable_variant execute_statements(list *statements, dict *values, dict *callables) {
    the_values = values;
    the_callables = callables;
    bool should_break = false;
    bool should_continue = false;
    bool should_return = false;
    failable_variant execution = execute_statements_once(statements, &should_break, &should_continue, &should_return);
    if (execution.failed) return failed_variant("%s", execution.err_msg);
    return ok_variant(execution.result);
}

static failable_bool check_condition(expression *condition) {
    failable_variant exec = execute_expression(condition, the_values, the_callables);
    if (exec.failed) return failed_bool("%s", exec.err_msg);
    if (!variant_is_bool(exec.result)) return failed_bool("condition must result in a boolean value");
    return ok_bool(variant_as_bool(exec.result));
}

static failable_variant execute_single_statement(statement *stmt, bool *should_break, bool *should_continue, bool *should_return) {
    statement_type s_type = statement_get_type(stmt);
    failable_variant execution;
    variant *return_value = new_null_variant();

    if (s_type == ST_IF) {
        expression *condition = statement_get_expression(stmt, 0);
        failable_bool pass_check = check_condition(condition);
        if (pass_check.failed) return failed_variant("%s", pass_check.err_msg);
        if (pass_check.result) {
            execution = execute_statements_once(statement_get_statements_body(stmt, false), should_break, should_continue, should_return);
        } else if (statement_has_alternate_body(stmt)) { 
            execution = execute_statements_once(statement_get_statements_body(stmt, true), should_break, should_continue, should_return);
        } else {
            execution = ok_variant(return_value); // nothing to execute.
        }
        if (execution.failed)
            return failed_variant("%s", execution.err_msg);
        return_value = execution.result;
        
    } else if (s_type == ST_WHILE) {
        execution = execute_statements_in_loop(
            statement_get_expression(stmt, 0),
            statement_get_statements_body(stmt, false),
            NULL,
            should_return
        );
        if (execution.failed)
            return failed_variant("%s", execution.err_msg);
        return_value = execution.result;

    } else if (s_type == ST_FOR_LOOP) {
        execution = execute_expression(statement_get_expression(stmt, 0), the_values, the_callables);
        if (execution.failed)
            return failed_variant("%s", execution.err_msg);
        execution = execute_statements_in_loop(
            statement_get_expression(stmt, 1),
            statement_get_statements_body(stmt, false),
            statement_get_expression(stmt, 2),
            should_return
        );
        if (execution.failed)
            return failed_variant("%s", execution.err_msg);
        return_value = execution.result;

    } else if (s_type == ST_EXPRESSION) {
        execution = execute_expression(statement_get_expression(stmt, 0), the_values, the_callables);
        if (execution.failed)
            return failed_variant("%s", execution.err_msg);
        return_value = execution.result;

    } else if (s_type == ST_BREAK) {
        *should_break = true;
    } else if (s_type == ST_CONTINUE) {
        *should_continue = true;
    } else if (s_type == ST_RETURN) {
        expression *ret_val_expr = statement_get_expression(stmt, 0);
        if (ret_val_expr != NULL) {
            execution = execute_expression(ret_val_expr, the_values, the_callables);
            if (execution.failed)
                return failed_variant("%s", execution.err_msg);
            return_value = execution.result;
        } else {
            return_value = new_null_variant();
        }
        *should_return = true;
        return_value = execution.result;

    } else if (s_type == ST_FUNCTION) {
        // here we should register the function to a callable registry
    } else {
        return failed_variant("was expecting if, while, for, expression, return, but got %s", statement_to_string(stmt));
    }

    return ok_variant(return_value);
}


static failable_variant execute_statements_once(list *statements, bool *should_break, bool *should_continue, bool *should_return) {
    variant *return_value = new_null_variant();

    for_list(statements, it, statement, stmt) {
        failable_variant execution = execute_single_statement(stmt, should_break, should_continue, should_return);
        if (execution.failed) 
            return failed_variant("%s", execution.err_msg);
        return_value = execution.result;
        if (*should_break || *should_continue || *should_return)
            break;
    }

    return ok_variant(return_value);
}

static failable_variant execute_statements_in_loop(expression *pre_condition, list *statements, expression *next, bool *should_return) {
    iterator *it = list_iterator(statements);
    variant *return_value = new_null_variant();
    failable_variant execution;

    while (true) {
        failable_bool cond_check = check_condition(pre_condition);
        if (cond_check.failed) return failed_variant("%s", cond_check.err_msg);
        if (!cond_check.result) break;

        bool should_break = false;
        bool should_continue = false;
    
        for_iterator(it, statement, stmt) {
            execution = execute_single_statement(stmt, &should_break, &should_continue, should_return);
            if (execution.failed) return failed_variant("%s", execution.err_msg);
            return_value = execution.result;
            if (should_break || should_continue || *should_return)
                break;
        }

        // "continue" in "for" statements allows the "next" operation to run
        if (should_break) break;
        if (*should_return) return ok_variant(return_value);

        if (next != NULL) {
            execution = execute_expression(next, the_values, the_callables);
            if (execution.failed) return failed_variant("%s", execution.err_msg);
        }
    }

    return ok_variant(return_value);
}

