#include <stdlib.h>
#include <string.h>
#include "../../debugger/debugger.h"
#include "../../utils/str.h"
#include "../../utils/str_builder.h"
#include "execution_outcome.h"
#include "expression_execution.h"
#include "statement_execution.h"
#include "../built_ins/built_in_funcs.h"

// used for pre/post increment/decrement
static expression *one = NULL;

enum modify_and_store {
    MAS_ADD, MAS_SUB, MAS_MUL, MAS_DIV, MAS_MOD,
    MAS_RSH, MAS_LSH, MAS_AND, MAS_OR, MAS_XOR,
};
enum comparison { 
    COMP_GT, COMP_GE, 
    COMP_LT, COMP_LE, 
    COMP_EQ, COMP_NE
};

static execution_outcome modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx);
static execution_outcome retrieve_value(expression *e, exec_context *ctx, variant **this_value);
static execution_outcome store_value(expression *lvalue, exec_context *ctx, variant *rvalue);
static execution_outcome retrieve_element(expression *list_exp, expression *index_exp, exec_context *ctx);
static execution_outcome retrieve_member(expression *object, expression *member, exec_context *ctx, variant **this_value);
static execution_outcome make_function_call(expression *callable_expr, expression *args_expr, exec_context *ctx);

static execution_outcome calculate_unary_expression(expression *op_expr, variant *value, exec_context *ctx);
static execution_outcome calculate_binary_expression(expression *op_expr, variant *v1, variant *v2, exec_context *ctx);
static execution_outcome calculate_comparison(expression *op_expr, enum comparison cmp, variant *v1, variant *v2);

static execution_outcome expression_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx);



void initialize_expression_execution() {
    // used for inc/dec operations
    one = new_numeric_literal_expression("1", NULL);
}

execution_outcome execute_expression(expression *e, exec_context *ctx) {
    // first concern is whether the expression stores data, or is read only
    expression_type et = e->type;
    operator_type op = e->op;
    expression *lval_expr;
    expression *rval_expr;

    if (should_start_debugger(NULL, e, ctx)) {
        failable session = run_debugger(NULL, e, ctx);
        if (session.failed) return failed_outcome("%s", session.err_msg);
    }

    if (et == ET_UNARY_OP) {
        lval_expr = e->per_type.operation.operand1;
        switch (op) {
            case OP_PRE_INC:  return modify_and_store(lval_expr, MAS_ADD, one, false, ctx);
            case OP_PRE_DEC:  return modify_and_store(lval_expr, MAS_SUB, one, false, ctx);
            case OP_POST_INC: return modify_and_store(lval_expr, MAS_ADD, one, true, ctx);
            case OP_POST_DEC: return modify_and_store(lval_expr, MAS_SUB, one, true, ctx);
        }

    } else if (et == ET_BINARY_OP) {
        lval_expr = e->per_type.operation.operand1;
        rval_expr = e->per_type.operation.operand2;
        switch (op) {
            case OP_ASSIGNMENT:
                execution_outcome retrieval = retrieve_value(rval_expr, ctx, NULL);
                if (retrieval.exception_thrown || retrieval.failed) return retrieval;
                execution_outcome storage = store_value(lval_expr, ctx, retrieval.result);
                if (retrieval.exception_thrown || retrieval.failed) return retrieval;
                return retrieval;
            
            case OP_ADD_ASSIGN: return modify_and_store(lval_expr, MAS_ADD, rval_expr, false, ctx);
            case OP_SUB_ASSIGN: return modify_and_store(lval_expr, MAS_SUB, rval_expr, false, ctx);
            case OP_MUL_ASSIGN: return modify_and_store(lval_expr, MAS_MUL, rval_expr, false, ctx);
            case OP_DIV_ASSIGN: return modify_and_store(lval_expr, MAS_DIV, rval_expr, false, ctx);
            case OP_MOD_ASSIGN: return modify_and_store(lval_expr, MAS_MOD, rval_expr, false, ctx);
            case OP_RSH_ASSIGN: return modify_and_store(lval_expr, MAS_RSH, rval_expr, false, ctx);
            case OP_LSH_ASSIGN: return modify_and_store(lval_expr, MAS_LSH, rval_expr, false, ctx);
            case OP_AND_ASSIGN: return modify_and_store(lval_expr, MAS_AND, rval_expr, false, ctx);
            case OP_OR_ASSIGN:  return modify_and_store(lval_expr, MAS_OR,  rval_expr, false, ctx);
            case OP_XOR_ASSIGN: return modify_and_store(lval_expr, MAS_XOR, rval_expr, false, ctx);
        }
    }

    // all other expression types are not storing values
    return retrieve_value(e, ctx, NULL);
}

static execution_outcome retrieve_value(expression *e, exec_context *ctx, variant **this_value) {
    execution_outcome ex;
    operator_type op = e->op;
    const char *data = e->per_type.terminal_data;
    expression *operand1, *operand2;

    switch (e->type) {
        case ET_IDENTIFIER:
            variant *v = exec_context_resolve_symbol(ctx, data);
            if (v == NULL) {
                return exception_outcome(new_exception_variant(e->token->filename, e->token->line_no, e->token->column_no, NULL,
                    "identifier '%s' not found", data));
            }
            return ok_outcome(v);
        case ET_NUMERIC_LITERAL:
            return ok_outcome(new_int_variant(atoi(data)));
        case ET_STRING_LITERAL:
            return ok_outcome(new_str_variant(data));
        case ET_BOOLEAN_LITERAL:
            return ok_outcome(new_bool_variant(strcmp(data, "true") == 0));

        case ET_LIST_DATA:
            list *expressions_list = e->per_type.list_;
            variant *values_list = new_list_variant();
            for_list(expressions_list, list_iter, expression, list_exp) {
                ex = execute_expression(list_exp, ctx);
                if (ex.exception_thrown || ex.failed) return ex;
                variant *item = ex.result;
                list_variant_append(values_list, item);
                variant_drop_ref(item);
            }
            return ok_outcome(values_list);

        case ET_DICT_DATA:
            dict *expressions_dict = e->per_type.dict_;
            variant *values_dict = new_dict_variant();
            iterator *keys_it = dict_keys_iterator(expressions_dict);
            for_iterator(keys_it, str, key) {
                ex = execute_expression(dict_get(expressions_dict, key), ctx);
                if (ex.exception_thrown || ex.failed) return ex;
                variant *vkey = new_str_variant(key);
                variant *vitem = ex.result;
                dict_variant_set(values_dict, vkey, vitem);
                variant_drop_ref(vkey);
                variant_drop_ref(vitem);
            }
            return ok_outcome(values_dict);

        case ET_UNARY_OP:
            operand1 = e->per_type.operation.operand1;
            ex = execute_expression(operand1, ctx);
            if (ex.exception_thrown || ex.failed) return ex;
            return calculate_unary_expression(e, ex.result, ctx);

        case ET_BINARY_OP:
            op = e->op;
            operand1 = e->per_type.operation.operand1;
            operand2 = e->per_type.operation.operand2;

            if (op == OP_ARRAY_SUBSCRIPT) {
                return retrieve_element(operand1, operand2, ctx);
            } else if (op == OP_MEMBER) {
                return retrieve_member(operand1, operand2, ctx, this_value);
            } else if (op == OP_FUNC_CALL) {
                return make_function_call(operand1, operand2, ctx);
            } else {
                ex = execute_expression(operand1, ctx);
                if (ex.exception_thrown || ex.failed) return ex;
                variant *v1 = ex.result;
                ex = execute_expression(operand2, ctx);
                if (ex.exception_thrown || ex.failed) return ex;
                variant *v2 = ex.result;
                return calculate_binary_expression(e, v1, v2, ctx);
            }
            break;
        
        case ET_FUNC_DECL:
            // "retrieving" a `function () { ...}` expression
            // merely creates and returns a callable variant
            return ok_outcome(new_callable_variant(new_callable(
                "(user anonymous expression function)",
                expression_function_callable_executor, e
            )));
    }

    return exception_outcome(new_exception_variant(e->token->filename, e->token->line_no, e->token->column_no, NULL,
        "Cannot retrieve value, unknown expression / operator type"));
}

static execution_outcome modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx) {
    execution_outcome retrieval;
    int original_int = 0;
    int operand_int;
    int result_int;

    // for now we allow variable creation via simple assignment
    retrieval = retrieve_value(lvalue, ctx, NULL);
    if (retrieval.exception_thrown || retrieval.failed) return retrieval;
    variant *original = retrieval.result;
    if (!variant_instance_of(original, int_type))
        return failed_outcome("modify_and_store() should be called for integers only");
    original_int = int_variant_as_int(original);

    retrieval = retrieve_value(rvalue, ctx, NULL);
    if (retrieval.exception_thrown || retrieval.failed) return retrieval;
    variant *operand = retrieval.result;
    if (!variant_instance_of(operand, int_type))
        return failed_outcome("modify_and_store() should be called for integers only");
    operand_int = int_variant_as_int(operand);

    switch (op) {
        case MAS_ADD: result_int = original_int + operand_int; break;
        case MAS_SUB: result_int = original_int - operand_int; break;
        case MAS_MUL: result_int = original_int * operand_int; break;
        case MAS_DIV: 
            if (operand_int == 0)
                return exception_outcome(new_exception_variant(rvalue->token->filename, rvalue->token->line_no, rvalue->token->column_no, NULL,
                    "division by zero not possible with integers"));
            result_int = original_int / operand_int;
            break;
        case MAS_MOD: result_int = original_int  % operand_int; break;
        case MAS_RSH: result_int = original_int >> operand_int; break;
        case MAS_LSH: result_int = original_int << operand_int; break;
        case MAS_AND: result_int = original_int  & operand_int; break;
        case MAS_OR : result_int = original_int  | operand_int; break;
        case MAS_XOR: result_int = original_int  ^ operand_int; break;
    }
    
    variant *result = new_int_variant(result_int);
    execution_outcome storing = store_value(lvalue, ctx, result);
    if (storing.exception_thrown || storing.failed) return storing;

    return ok_outcome(return_original ? original : result);
}

static execution_outcome store_value(expression *lvalue, exec_context *ctx, variant *rvalue) {

    expression_type et = lvalue->type;
    if (et == ET_IDENTIFIER) {
        const char *name = lvalue->per_type.terminal_data;
        if (exec_context_symbol_exists(ctx, name))
            exec_context_update_symbol(ctx, name, rvalue);
        else
            exec_context_register_symbol(ctx, name, rvalue);
        return ok_outcome(NULL);

    } else if (et == ET_BINARY_OP) {
        operator_type op = lvalue->op;
        if (op == OP_ARRAY_SUBSCRIPT) {
            // e.g. "ARRAY_SUBSCRIPT(<list_like_executionable>, <int_like_executionable>)"
            expression *op1 = lvalue->per_type.operation.operand1;
            execution_outcome op1_exec = execute_expression(op1, ctx);
            if (op1_exec.exception_thrown || op1_exec.failed) return op1_exec;
            if (!variant_instance_of(op1_exec.result, list_type))
                return exception_outcome(new_exception_variant(op1->token->filename, op1->token->line_no, op1->token->column_no, NULL,
                    "array subscripts apply only to lists"));

            expression *op2 = lvalue->per_type.operation.operand2;
            execution_outcome op2_exec = execute_expression(op2, ctx);
            if (op2_exec.exception_thrown || op2_exec.failed) return op2_exec;
            if (!variant_instance_of(op2_exec.result, int_type))
                return exception_outcome(new_exception_variant(op2->token->filename, op2->token->line_no, op2->token->column_no, NULL,
                    "only integer expression can be used as list indices"));

            list *l = list_variant_as_list(op1_exec.result);
            int i = int_variant_as_int(op2_exec.result);
            list_set(l, i, rvalue);
            return ok_outcome(NULL);

        } else if (op == OP_MEMBER) {
            expression *op1 = lvalue->per_type.operation.operand1;
            execution_outcome op1_exec = execute_expression(op1, ctx);
            if (op1_exec.exception_thrown || op1_exec.failed) return op1_exec;
            if (!variant_instance_of(op1_exec.result, dict_type))
                return exception_outcome(new_exception_variant(op1->token->filename, op1->token->line_no, op1->token->column_no, NULL,
                    "identifier member storage works only with dictionaries"));

            expression *op2 = lvalue->per_type.operation.operand2;
            if (op2->type != ET_IDENTIFIER)
                return exception_outcome(new_exception_variant(op1->token->filename, op1->token->line_no, op1->token->column_no, NULL,
                    "only identifiers can be used for dictionary member storage"));

            dict *d = dict_variant_as_dict(op1_exec.result);
            const char *key = op2->per_type.terminal_data;
            dict_set(d, key, rvalue);
            return ok_outcome(NULL);

        } else {
            return exception_outcome(new_exception_variant(lvalue->token->filename, lvalue->token->line_no, lvalue->token->column_no, NULL,
                "operator type cannot be used as lvalue: %s", operator_type_name(op)));
        }
        
    } else {
        str_builder *sb = new_str_builder();
        expression_describe(lvalue, sb);
        return exception_outcome(new_exception_variant(lvalue->token->filename, lvalue->token->line_no, lvalue->token->column_no, NULL,
            "expression cannot be used as lvalue: %s", str_builder_charptr(sb)));
    }
}

static execution_outcome calculate_comparison(expression *op_expr, enum comparison cmp, variant *v1, variant *v2) {

    if (variant_instance_of(v1, int_type) && variant_instance_of(v2, int_type)) {
        int i1 = int_variant_as_int(v1);
        int i2 = int_variant_as_int(v2);
        switch (cmp) {
            case COMP_GT: return ok_outcome(new_bool_variant(i1 >  i2));
            case COMP_GE: return ok_outcome(new_bool_variant(i1 >= i2));
            case COMP_LT: return ok_outcome(new_bool_variant(i1 <  i2));
            case COMP_LE: return ok_outcome(new_bool_variant(i1 <= i2));
            case COMP_EQ: return ok_outcome(new_bool_variant(i1 == i2));
            case COMP_NE: return ok_outcome(new_bool_variant(i1 != i2));
        }
    } else if (variant_instance_of(v1, float_type) && variant_instance_of(v2, float_type)) {
        float f1 = float_variant_as_float(v1);
        float f2 = float_variant_as_float(v2);
        switch (cmp) {
            case COMP_GT: return ok_outcome(new_bool_variant(f1 >  f2));
            case COMP_GE: return ok_outcome(new_bool_variant(f1 >= f2));
            case COMP_LT: return ok_outcome(new_bool_variant(f1 <  f2));
            case COMP_LE: return ok_outcome(new_bool_variant(f1 <= f2));
            case COMP_EQ: return ok_outcome(new_bool_variant(f1 == f2));
            case COMP_NE: return ok_outcome(new_bool_variant(f1 != f2));
        }
    } else if (variant_instance_of(v1, str_type) && variant_instance_of(v2, str_type)) {
        int c = variant_compare(v1, v2);
        switch (cmp) {
            case COMP_GT: return ok_outcome(new_bool_variant(c >  0));
            case COMP_GE: return ok_outcome(new_bool_variant(c >= 0));
            case COMP_LT: return ok_outcome(new_bool_variant(c <  0));
            case COMP_LE: return ok_outcome(new_bool_variant(c <= 0));
            case COMP_EQ: return ok_outcome(new_bool_variant(c == 0));
            case COMP_NE: return ok_outcome(new_bool_variant(c != 0));
        }
    } else if (variant_instance_of(v1, bool_type) && variant_instance_of(v2, bool_type)) {
        bool b1 = bool_variant_as_bool(v1);
        bool b2 = bool_variant_as_bool(v2);
        switch (cmp) {
            case COMP_EQ: return ok_outcome(new_bool_variant(b1 == b2));
            case COMP_NE: return ok_outcome(new_bool_variant(b1 != b2));
        }
    } else if (variant_instance_of(v1, list_type) && variant_instance_of(v2, list_type)) {
        list *l1 = list_variant_as_list(v1);
        list *l2 = list_variant_as_list(v2);
        bool eq = lists_are_equal(l1, l2);
        switch (cmp) {
            case COMP_EQ: return ok_outcome(new_bool_variant(eq));
            case COMP_NE: return ok_outcome(new_bool_variant(!eq));
        }
    }

    variant *s1 = variant_to_string(v1);
    variant *s2 = variant_to_string(v2);
    variant *exception = new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
        "cannot compare given operands: %s, %s",
        str_variant_as_str(s1), str_variant_as_str(s2));
    variant_drop_ref(s1);
    variant_drop_ref(s2);
    return exception_outcome(exception);
}

static execution_outcome retrieve_element(expression *list_exp, expression *index_exp, exec_context *ctx) {

    execution_outcome ex = execute_expression(list_exp, ctx);
    if (ex.exception_thrown || ex.failed) return ex;
    if (!variant_instance_of(ex.result, list_type)) 
        return exception_outcome(new_exception_variant(
            list_exp->token->filename, list_exp->token->line_no, list_exp->token->column_no, NULL,
            "ARRAY_SUBSCRIPT requires a list as left operand"
        ));
    list *l = list_variant_as_list(ex.result);

    ex = execute_expression(index_exp, ctx);
    if (ex.exception_thrown || ex.failed) return ex;
    variant *subscript = ex.result;
    if (!variant_instance_of(subscript, int_type))
        return exception_outcome(new_exception_variant(
            index_exp->token->filename, index_exp->token->line_no, index_exp->token->column_no, NULL,
            "ARRAY_SUBSCRIPT requires an integer as right operand"
        ));
    int index = int_variant_as_int(subscript);

    if (index < 0 || index >= list_length(l))
        return exception_outcome(new_exception_variant(
            index_exp->token->filename, index_exp->token->line_no, index_exp->token->column_no, NULL,
            "array index (%d) out of bounds (0..%d)", index, list_length(l)
        ));
    return ok_outcome(list_get(l, index));
}

static execution_outcome retrieve_member(expression *obj_exp, expression *mbr_exp, exec_context *ctx, variant **this_value) {

    /* some objects, like a list, may behave as a dict,
    in the sense of methods, e.g. "list1.length()", or "items.add(item)" */
    execution_outcome ex = execute_expression(obj_exp, ctx);
    if (ex.exception_thrown || ex.failed) return ex;
    variant *object = ex.result;
    if (this_value != NULL) // save for caller
        *this_value = object;
    
    if (mbr_exp->type != ET_IDENTIFIER)
        return exception_outcome(new_exception_variant(
            mbr_exp->token->filename, mbr_exp->token->line_no, mbr_exp->token->column_no, NULL,
            "MEMBER_OF requires identifier as right operand"
        ));
    const char *member = mbr_exp->per_type.terminal_data;
    
    dict *d = NULL;
    if (variant_instance_of(object, dict_type)) {
        // find member in normal members, fallback to built-it methods
        d = dict_variant_as_dict(object);
        if (dict_has(d, member))
            return ok_outcome(dict_get(d, member));
        
        d = get_built_in_dict_methods_dictionary();
        if (dict_has(d, member))
            return ok_outcome(dict_get(d, member));

    } else if (variant_instance_of(object, list_type)) {
        d = get_built_in_list_methods_dictionary();
        if (dict_has(d, member))
            return ok_outcome(dict_get(d, member));
        
    } else if (variant_instance_of(object, str_type)) {
        d = get_built_in_str_methods_dictionary();
        if (dict_has(d, member))
            return ok_outcome(dict_get(d, member));
    }

    return exception_outcome(new_exception_variant(
        mbr_exp->token->filename, mbr_exp->token->line_no, mbr_exp->token->column_no, NULL,
        "member '%s' not found in object / dict", member
    ));
}

static execution_outcome make_function_call(expression *target_exp, expression *args_expr, exec_context *ctx) {

    variant *this_value = NULL;
    execution_outcome retrieval = retrieve_value(target_exp, ctx, &this_value);
    if (retrieval.exception_thrown || retrieval.failed) return retrieval;
    variant *target = retrieval.result;

    if (target == NULL)
        return exception_outcome(new_exception_variant(
            target_exp->token->filename, target_exp->token->line_no, target_exp->token->column_no, NULL,
            "could not resolve call target"
        ));
    if (!variant_instance_of(target, callable_type))
        return exception_outcome(new_exception_variant(
            target_exp->token->filename, target_exp->token->line_no, target_exp->token->column_no, NULL,
            "call target is not callable"
        ));
    callable *c = callable_variant_as_callable(target);

    retrieval = retrieve_value(args_expr, ctx, NULL);
    if (retrieval.exception_thrown || retrieval.failed) return retrieval;
    list *arg_values = list_variant_as_list(retrieval.result);

    return callable_call(c, arg_values, NULL, this_value, ctx);
}

static execution_outcome calculate_unary_expression(expression *op_expr, variant *value, exec_context *ctx) {
    switch (op_expr->op) {
        case OP_POSITIVE_NUM:
            if (variant_instance_of(value, int_type) || variant_instance_of(value, float_type))
                return ok_outcome(value);
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "positive num only works for int / float values"
            ));

        case OP_NEGATIVE_NUM:
            if (variant_instance_of(value, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(value) * -1));
            if (variant_instance_of(value, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(value) * -1));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "negative num only works for int / float values"
            ));

        case OP_LOGICAL_NOT:
            // let's avoid implicit conversion to bool for now.
            if (variant_instance_of(value, bool_type))
                return ok_outcome(new_bool_variant(!bool_variant_as_bool(value)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "logical-not only works for bool values"
            ));

        case OP_BITWISE_NOT:
            if (variant_instance_of(value, int_type))
                return ok_outcome(new_int_variant(~int_variant_as_int(value)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "bitwise-not only works for int values"
            ));
    }

    return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
        "Unknown unary operator type %s", operator_type_name(op_expr->type)));
}

static execution_outcome calculate_binary_expression(expression *op_expr, variant *v1, variant *v2, exec_context *ctx) {
    switch (op_expr->op) {
        case OP_MULTIPLY:
            if (variant_instance_of(v1, int_type) && variant_instance_of(v2, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) * int_variant_as_int(v2)));
            if (variant_instance_of(v1, float_type) && variant_instance_of(v2, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(v1) * float_variant_as_float(v2)));
            if (variant_instance_of(v1, str_type) && variant_instance_of(v2, int_type)) {
                str_builder *tmp = new_str_builder();
                for (int i = 0; i < int_variant_as_int(v2); i++)
                    str_builder_add(tmp, deprecated_variant_as_const_char(v1));
                return ok_outcome(new_str_variant(str_builder_charptr(tmp)));
            }
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "multiplication is only supported in int/float types"
            ));

        case OP_DIVIDE:
            if (variant_instance_of(v1, int_type)) {
                int denominator = int_variant_as_int(v2);
                if (denominator == 0)
                    return exception_outcome(new_exception_variant(
                        op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                        "division by zero not possible in integers"
                    ));
                return ok_outcome(new_int_variant(int_variant_as_int(v1) / denominator));
            }
            if (variant_instance_of(v1, float_type)) {
                // in floats, the result is "infinity"
                return ok_outcome(new_float_variant(float_variant_as_float(v1) / float_variant_as_float(v2)));
            }
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "division is only supported in int/float types"
            ));

        case OP_MODULO:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) % int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "modulo is only supported in int types"
            ));

        case OP_ADD:
            if (variant_instance_of(v1, int_type) && variant_instance_of(v2, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) + int_variant_as_int(v2)));
            if (variant_instance_of(v1, float_type) && variant_instance_of(v2, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(v1) + float_variant_as_float(v2)));
            if (variant_instance_of(v1, str_type)) {
                str_builder *sb = new_str_builder();
                str_builder_add(sb, deprecated_variant_as_const_char(v1));
                str_builder_add(sb, deprecated_variant_as_const_char(v2));
                variant *v = new_str_variant(strdup(str_builder_charptr(sb)));
                str_builder_free(sb);
                return ok_outcome(v);
            }
            // how about adding items to a list???
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "addition is only supported in int, float, string types"
            ));

        case OP_SUBTRACT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) - int_variant_as_int(v2)));
            if (variant_instance_of(v1, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(v1) - float_variant_as_float(v2)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "subtraction is only supported in int/float types"
            ));

        case OP_LSHIFT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) << int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "left shift is only supported in int types"
            ));

        case OP_RSHIFT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) >> int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(
                op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "right shift is only supported in int types"
            ));

        case OP_LESS_THAN:      return calculate_comparison(op_expr, COMP_LT, v1, v2);
        case OP_LESS_EQUAL:     return calculate_comparison(op_expr, COMP_LE, v1, v2);
        case OP_GREATER_THAN:   return calculate_comparison(op_expr, COMP_GT, v1, v2);
        case OP_GREATER_EQUAL:  return calculate_comparison(op_expr, COMP_GE, v1, v2);
        case OP_EQUAL:          return calculate_comparison(op_expr, COMP_EQ, v1, v2);
        case OP_NOT_EQUAL:      return calculate_comparison(op_expr, COMP_NE, v1, v2);

        case OP_BITWISE_AND:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) & int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "bitwise operations only supported in int types"));

        case OP_BITWISE_XOR:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) ^ int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "bitwise operations only supported in int types"));

        case OP_BITWISE_OR:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) | int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "bitwise operations only supported in int types"));

        case OP_LOGICAL_AND:
            // we could do shorthand here...
            if (variant_instance_of(v1, bool_type) && variant_instance_of(v2, bool_type))
                return ok_outcome(new_bool_variant(bool_variant_as_bool(v1) && bool_variant_as_bool(v2)));
            return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "logical operations only supported in bool types"));
            
        case OP_LOGICAL_OR:
            // we could do shorthand here...
            if (variant_instance_of(v1, bool_type) && variant_instance_of(v2, bool_type))
                return ok_outcome(new_bool_variant(bool_variant_as_bool(v1) || bool_variant_as_bool(v2)));
            return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                "logical operations only supported in bool types"));

        case OP_SHORT_IF:
            if (!variant_instance_of(v1, bool_type))
                return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
                    "shorthand-if operator_type requires boolean condition"));
            bool passed = bool_variant_as_bool(v1);
            if (!variant_instance_of(v2, list_type))
                return failed_outcome("shorthand-if operator_type was expecting a list of 2 arguments, maybe parsing has a bug?");
            list *values_pair = list_variant_as_list(v2);
            if (list_length(values_pair) != 2)
                return failed_outcome("shorthand-if operator_type was expecting exactly two arguments in the list, maybe parsing has a bug?");
            return ok_outcome(list_get(values_pair, passed ? 0 : 1));
    }

    return exception_outcome(new_exception_variant(op_expr->token->filename, op_expr->token->line_no, op_expr->token->column_no, NULL,
        "Unknown binary operator type %s", operator_type_name(op_expr->type)));
}

static execution_outcome expression_function_callable_executor(
    list *positional_args, 
    dict *named_args, 
    void *callable_data, 
    variant *this_obj,
    exec_context *ctx
) {
    expression *expr = (expression *)callable_data;

    list *arg_names = expr->per_type.func.arg_names;
    if (list_length(positional_args) < list_length(arg_names)) {
        // we should report where the call was made, not where the function is
        return exception_outcome(new_exception_variant(
            expr->token->filename, expr->token->line_no, expr->token->column_no, NULL,
            "%s() expected %d arguments, got %d", expr->per_type.func.name, list_length(arg_names), list_length(positional_args)
        ));
    }

    stack_frame *frame = new_stack_frame(expr->per_type.func.name, NULL, expr);
    stack_frame_initialization(frame, arg_names, positional_args, named_args, this_obj);
    exec_context_push_stack_frame(ctx, frame);

    execution_outcome result = execute_statements(expr->per_type.func.statements, ctx);
    
    // even if an exception was raised, we still must pop the stack frame
    exec_context_pop_stack_frame(ctx);

    return result;
}
