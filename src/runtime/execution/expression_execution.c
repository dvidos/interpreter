#include <stdlib.h>
#include <string.h>
#include "../variants/_variants.h"
#include "../../debugger/debugger.h"
#include "../../utils/data_types/callable.h"
#include "../../utils/cstr.h"
#include "../../utils/str.h"
#include "expression_execution.h"
#include "statement_execution.h"
#include "function_execution.h"
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
static execution_outcome retrieve_value(expression *e, exec_context *ctx);
static execution_outcome store_value(expression *lvalue, exec_context *ctx, variant *rvalue);

static execution_outcome retrieve_element(expression *list_exp, expression *index_exp, exec_context *ctx);
static execution_outcome store_element(expression *container_expr, expression *element_expr, variant *value, exec_context *ctx);

static execution_outcome retrieve_member(expression *object, expression *member, exec_context *ctx);
static execution_outcome store_member(expression *container_expr, expression *member_expr, variant *value, exec_context *ctx);
static execution_outcome call_member(expression *container_expr, expression *member_expr, expression *args_expr, origin *call_origin, exec_context *ctx);

static execution_outcome make_function_call(expression *call_target_expr, expression *args_expr, origin *call_origin, exec_context *ctx);
static execution_outcome calculate_unary_expression(expression *op_expr, variant *value, exec_context *ctx);
static execution_outcome calculate_binary_expression(expression *op_expr, variant *v1, variant *v2, exec_context *ctx);
static execution_outcome calculate_comparison(expression *op_expr, enum comparison cmp, variant *v1, variant *v2);
static execution_outcome expression_function_callable_executor(
    list *arg_values, 
    void *ast_node, 
    variant *this_obj,
    dict *captured_values, // optional for closures
    origin *call_origin, // source of call
    exec_context *ctx);



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
                execution_outcome retrieval = retrieve_value(rval_expr, ctx);
                if (retrieval.exception_thrown || retrieval.failed) return retrieval;
                execution_outcome storage = store_value(lval_expr, ctx, retrieval.result);
                if (storage.exception_thrown || storage.failed) return storage;
                return retrieval; // assignment returns the assigned value
            
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
    return retrieve_value(e, ctx);
}

static execution_outcome retrieve_value(expression *e, exec_context *ctx) {
    execution_outcome ex;
    operator_type op = e->op;
    const char *data = e->per_type.terminal_data;
    expression *operand1, *operand2;

    switch (e->type) {
        case ET_IDENTIFIER:
            variant *v = exec_context_resolve_symbol(ctx, data);
            if (v == NULL) {
                return exception_outcome(new_exception_variant_at(e->token->origin, NULL,
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
            list *values_list = new_list(variant_item_info);
            for_list(expressions_list, list_iter, expression, list_exp) {
                ex = execute_expression(list_exp, ctx);
                if (ex.excepted || ex.failed) return ex;
                list_add(values_list, ex.result);
            }
            return ok_outcome(new_list_variant_owning(values_list));

        case ET_DICT_DATA:
            dict *expressions_dict = e->per_type.dict_;
            dict *values_dict = new_dict(variant_item_info);
            iterator *keys_it = dict_keys_iterator(expressions_dict);
            for_iterator(keys_it, cstr, key) {
                ex = execute_expression(dict_get(expressions_dict, key), ctx);
                if (ex.excepted || ex.failed) return ex;
                dict_set(values_dict, key, ex.result);
            }
            return ok_outcome(new_dict_variant_owning(values_dict));

        case ET_UNARY_OP:
            operand1 = e->per_type.operation.operand1;
            ex = execute_expression(operand1, ctx);
            if (ex.excepted || ex.failed) return ex;
            return calculate_unary_expression(e, ex.result, ctx);

        case ET_BINARY_OP:
            op = e->op;
            operand1 = e->per_type.operation.operand1;
            operand2 = e->per_type.operation.operand2;

            if (op == OP_ARRAY_SUBSCRIPT) {
                return retrieve_element(operand1, operand2, ctx);
            } else if (op == OP_MEMBER) {
                return retrieve_member(operand1, operand2, ctx);
            } else if (op == OP_FUNC_CALL) {
                return make_function_call(operand1, operand2, e->token->origin, ctx);
            } else {
                ex = execute_expression(operand1, ctx);
                if (ex.excepted || ex.failed) return ex;
                variant *v1 = ex.result;
                ex = execute_expression(operand2, ctx);
                if (ex.excepted || ex.failed) return ex;
                variant *v2 = ex.result;
                return calculate_binary_expression(e, v1, v2, ctx);
            }
            break;
        
        case ET_FUNC_DECL:
            // "retrieving" a `function () { ...}` expression merely creates and returns a callable variant
            return ok_outcome(new_callable_variant(
                new_callable(
                    "(user anonymous function)",
                    expression_function_callable_executor, 
                    e, NULL, NULL)));
    }

    return exception_outcome(new_exception_variant_at(e->token->origin, NULL,
        "Cannot retrieve value, unknown expression / operator type"));
}

static execution_outcome modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx) {
    execution_outcome retrieval;
    int original_int = 0;
    int operand_int;
    int result_int;

    // for now we allow variable creation via simple assignment
    retrieval = retrieve_value(lvalue, ctx);
    if (retrieval.exception_thrown || retrieval.failed) return retrieval;
    variant *original = retrieval.result;
    if (!variant_instance_of(original, int_type))
        return failed_outcome("modify_and_store() should be called for integers only");
    original_int = int_variant_as_int(original);

    retrieval = retrieve_value(rvalue, ctx);
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
                return exception_outcome(new_exception_variant_at(rvalue->token->origin, NULL,
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
            return store_element(lvalue->per_type.operation.operand1, lvalue->per_type.operation.operand2, rvalue, ctx);


        } else if (op == OP_MEMBER) {
            return store_member(lvalue->per_type.operation.operand1, lvalue->per_type.operation.operand2, rvalue, ctx);

        } else {
            return exception_outcome(new_exception_variant_at(lvalue->token->origin, NULL,
                "operator type cannot be used as lvalue: %s", operator_type_name(op)));
        }
        
    } else {
        str *str = new_str();
        expression_describe(lvalue, str);
        return exception_outcome(new_exception_variant_at(lvalue->token->origin, NULL,
            "expression cannot be used as lvalue: %s", str_cstr(str)));
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
    variant *exception = new_exception_variant_at(op_expr->token->origin, NULL,
        "cannot compare given operands: %s, %s",
        str_variant_as_str(s1), str_variant_as_str(s2));
    variant_drop_ref(s1);
    variant_drop_ref(s2);
    return exception_outcome(exception);
}

static execution_outcome retrieve_element(expression *container_expr, expression *element_expr, exec_context *ctx) {

    execution_outcome ex = execute_expression(container_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *container = ex.result;
    
    ex = execute_expression(element_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *element = ex.result;

    ex = variant_get_element(container, element);

    if (ex.result != NULL)
        variant_inc_ref(ex.result);
    
    return ex;
}

static execution_outcome store_element(expression *container_expr, expression *element_expr, variant *value, exec_context *ctx) {

    execution_outcome ex = execute_expression(container_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *container = ex.result;

    ex = execute_expression(element_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *element = ex.result;

    ex = variant_set_element(container, element, value);
    
    return ex;
}

static execution_outcome retrieve_member(expression *container_expr, expression *member_expr, exec_context *ctx) {

    execution_outcome ex = execute_expression(container_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *container = ex.result;

    visibility vis = exec_context_is_curr_method_owned_by(ctx, container->_type) ?
        VIS_SAME_CLASS_CODE : VIS_PUBLIC_CODE;

    if (member_expr->type != ET_IDENTIFIER)
        return exception_outcome(new_exception_variant("MEMBER_OF requires identifier as right operand"));
    const char *member = member_expr->per_type.terminal_data;

    if (variant_has_attr(container, member, vis)) {
        ex = variant_get_attr_value(container, member, vis);
        if (ex.excepted || ex.failed) return ex;
        variant *value = ex.result;
        variant_inc_ref(value);
        return ok_outcome(value);

    } else if (variant_has_method(container, member, vis)) {
        // promote a function to an instance, capturing the container as 'this'
        return variant_get_bound_method(container, member, vis);

    } else {
        return exception_outcome(new_exception_variant("member '%s' not found in object type '%s'",
            member, container->_type->name));
    }
}

static execution_outcome store_member(expression *container_expr, expression *member_expr, variant *value, exec_context *ctx) {

    execution_outcome ex = execute_expression(container_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *container = ex.result;

    visibility vis = exec_context_is_curr_method_owned_by(ctx, container->_type) ?
        VIS_SAME_CLASS_CODE : VIS_PUBLIC_CODE;

    if (member_expr->type != ET_IDENTIFIER)
        return exception_outcome(new_exception_variant("MEMBER_OF requires identifier as right operand"));
    const char *member = member_expr->per_type.terminal_data;

    if (!variant_has_attr(container, member, vis))
        return exception_outcome(new_exception_variant("attribute '%s' not found in object type '%s'",
            member, container->_type->name));

    return variant_set_attr_value(container, member, vis, value);
}

static execution_outcome call_member(expression *container_expr, expression *member_expr, expression *args_expr, origin *call_origin, exec_context *ctx) {

    execution_outcome ex = execute_expression(container_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    variant *container = ex.result;

    visibility vis = exec_context_is_curr_method_owned_by(ctx, container->_type) ?
        VIS_SAME_CLASS_CODE : VIS_PUBLIC_CODE;

    if (member_expr->type != ET_IDENTIFIER)
        return exception_outcome(new_exception_variant("MEMBER_OF requires identifier as right operand"));
    const char *member = member_expr->per_type.terminal_data;

    // arguments is a list of expressions, evaluating to a list of values
    if (args_expr->type != ET_LIST_DATA)
        return exception_outcome(new_exception_variant("function call requires a list of args"));
    
    ex = retrieve_value(args_expr, ctx);
    if (ex.excepted || ex.failed) return ex;
    list *args = list_variant_as_list(ex.result);

    if (variant_has_method(container, member, vis)) {
        return variant_call_method(container, member, vis, args, call_origin, ctx);

    } else if (variant_has_attr(container, member, vis)) {
        ex = variant_get_attr_value(container, member, vis);
        if (ex.excepted || ex.failed) return ex;

        // attribute could be an expression function or callable.
        return variant_call(ex.result, args, NULL, call_origin, ctx);

    } else {
        return exception_outcome(new_exception_variant("no callable member '%s' found in object type '%s'",
            member, container->_type->name));
    }
}

static execution_outcome make_function_call(expression *call_target_expr, expression *args_expr, origin *call_origin, exec_context *ctx) {

    if (call_target_expr->op == OP_MEMBER) {
        // if calling a member of something, avoid promoting the method 
        // to an instance, call on the object directly.
        // remember, object instances don't have func pointers, the class instance does.
        return call_member(call_target_expr->per_type.operation.operand1, call_target_expr->per_type.operation.operand2, args_expr, call_target_expr->token->origin, ctx);

    } else {
        // otherwise, derive the callable and call it.
        execution_outcome ex = retrieve_value(call_target_expr, ctx);
        if (ex.excepted || ex.failed) return ex;
        variant *call_target = ex.result;

        if (args_expr->type != ET_LIST_DATA)
            return exception_outcome(new_exception_variant("call requires a list of expressions"));
        ex = retrieve_value(args_expr, ctx);
        if (ex.excepted || ex.failed) return ex;
        list *args = list_variant_as_list(ex.result);

        ex = variant_call(call_target, args, NULL, call_target_expr->token->origin, ctx);
        return ex;
    }
}

static execution_outcome calculate_unary_expression(expression *op_expr, variant *value, exec_context *ctx) {
    switch (op_expr->op) {
        case OP_POSITIVE_NUM:
            if (variant_instance_of(value, int_type) || variant_instance_of(value, float_type))
                return ok_outcome(value);
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "positive num only works for int / float values"
            ));

        case OP_NEGATIVE_NUM:
            if (variant_instance_of(value, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(value) * -1));
            if (variant_instance_of(value, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(value) * -1));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "negative num only works for int / float values"
            ));

        case OP_LOGICAL_NOT:
            // let's avoid implicit conversion to bool for now.
            if (variant_instance_of(value, bool_type))
                return ok_outcome(new_bool_variant(!bool_variant_as_bool(value)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "logical-not only works for bool values"
            ));

        case OP_BITWISE_NOT:
            if (variant_instance_of(value, int_type))
                return ok_outcome(new_int_variant(~int_variant_as_int(value)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "bitwise-not only works for int values"
            ));
    }

    return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
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
                str *tmp = new_str();
                for (int i = 0; i < int_variant_as_int(v2); i++)
                    str_add(tmp, str_variant_as_str(v1));
                return ok_outcome(new_str_variant(str_cstr(tmp)));
            }
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "multiplication is only supported in int/float types"
            ));

        case OP_DIVIDE:
            if (variant_instance_of(v1, int_type)) {
                int denominator = int_variant_as_int(v2);
                if (denominator == 0)
                    return exception_outcome(new_exception_variant_at(
                        op_expr->token->origin, NULL,
                        "division by zero not possible in integers"
                    ));
                return ok_outcome(new_int_variant(int_variant_as_int(v1) / denominator));
            }
            if (variant_instance_of(v1, float_type)) {
                // in floats, the result is "infinity"
                return ok_outcome(new_float_variant(float_variant_as_float(v1) / float_variant_as_float(v2)));
            }
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "division is only supported in int/float types"
            ));

        case OP_MODULO:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) % int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "modulo is only supported in int types"
            ));

        case OP_ADD:
            if (variant_instance_of(v1, int_type) && variant_instance_of(v2, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) + int_variant_as_int(v2)));
            if (variant_instance_of(v1, float_type) && variant_instance_of(v2, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(v1) + float_variant_as_float(v2)));
            if (variant_instance_of(v1, str_type) && variant_instance_of(v2, str_type)) {
                str *str = new_str();
                str_add(str, str_variant_as_str(v1));
                str_add(str, str_variant_as_str(v2));
                variant *v = new_str_variant(strdup(str_cstr(str)));
                str_free(str);
                return ok_outcome(v);
            }
            // how about adding items to a list???
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "addition is only supported in int, float, string types"
            ));

        case OP_SUBTRACT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) - int_variant_as_int(v2)));
            if (variant_instance_of(v1, float_type))
                return ok_outcome(new_float_variant(float_variant_as_float(v1) - float_variant_as_float(v2)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "subtraction is only supported in int/float types"
            ));

        case OP_LSHIFT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) << int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
                "left shift is only supported in int types"
            ));

        case OP_RSHIFT:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) >> int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant_at(
                op_expr->token->origin, NULL,
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
            return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                "bitwise operations only supported in int types"));

        case OP_BITWISE_XOR:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) ^ int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                "bitwise operations only supported in int types"));

        case OP_BITWISE_OR:
            if (variant_instance_of(v1, int_type))
                return ok_outcome(new_int_variant(int_variant_as_int(v1) | int_variant_as_int(v2)));
            return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                "bitwise operations only supported in int types"));

        case OP_LOGICAL_AND:
            // we could do shorthand here...
            if (variant_instance_of(v1, bool_type) && variant_instance_of(v2, bool_type))
                return ok_outcome(new_bool_variant(bool_variant_as_bool(v1) && bool_variant_as_bool(v2)));
            return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                "logical operations only supported in bool types"));
            
        case OP_LOGICAL_OR:
            // we could do shorthand here...
            if (variant_instance_of(v1, bool_type) && variant_instance_of(v2, bool_type))
                return ok_outcome(new_bool_variant(bool_variant_as_bool(v1) || bool_variant_as_bool(v2)));
            return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                "logical operations only supported in bool types"));

        case OP_SHORT_IF:
            if (!variant_instance_of(v1, bool_type))
                return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
                    "shorthand-if operator_type requires boolean condition"));
            bool passed = bool_variant_as_bool(v1);
            if (!variant_instance_of(v2, list_type))
                return failed_outcome("shorthand-if operator_type was expecting a list of 2 arguments, maybe parsing has a bug?");
            list *values_pair = list_variant_as_list(v2);
            if (list_length(values_pair) != 2)
                return failed_outcome("shorthand-if operator_type was expecting exactly two arguments in the list, maybe parsing has a bug?");
            return ok_outcome(list_get(values_pair, passed ? 0 : 1));
    }

    return exception_outcome(new_exception_variant_at(op_expr->token->origin, NULL,
        "Unknown binary operator type %s", operator_type_name(op_expr->type)));
}

static execution_outcome expression_function_callable_executor(
    list *arg_values, 
    void *ast_node, 
    variant *this_obj,
    dict *captured_values, // optional for closures
    origin *call_origin, // source of call
    exec_context *ctx
) {
    expression *expr = (expression *)ast_node;

    list *arg_names = expr->per_type.func.arg_names;
    if (list_length(arg_values) < list_length(arg_names)) {
        // we should report where the call was made, not where the function is
        return exception_outcome(new_exception_variant_at(
            expr->token->origin, NULL,
            "%s() expected %d arguments, got %d", expr->per_type.func.name, list_length(arg_names), list_length(arg_values)
        ));
    }

    stack_frame *frame = new_stack_frame(expr->per_type.func.name, expr->token->origin);
    stack_frame_initialization(frame, arg_names, arg_values, this_obj);
    exec_context_push_stack_frame(ctx, frame);

    execution_outcome result = execute_statements(expr->per_type.func.statements, ctx);
    
    // even if an exception was raised, we still must pop the stack frame
    exec_context_pop_stack_frame(ctx);

    return result;
}
