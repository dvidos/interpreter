#include <stdlib.h>
#include <string.h>
#include "../utils/str.h"
#include "../utils/str_builder.h"
#include "expression_execution.h"
#include "statement_execution.h"
#include "built_in_funcs.h"

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

static failable_variant modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx);
static failable_variant retrieve_value(expression *e, exec_context *ctx, variant **this_value);
static failable         store_value(expression *lvalue, exec_context *ctx, variant *rvalue);
static failable_variant retrieve_member(expression *object, expression *member, exec_context *ctx, variant **this_value);
static failable_variant make_function_call(expression *callable_expr, expression *args_expr, exec_context *ctx);

static failable_variant calculate_unary_operation(operator_type op, variant *value, exec_context *ctx);
static failable_variant calculate_binary_operation(operator_type op, variant *v1, variant *v2, exec_context *ctx);
static failable_variant calculate_comparison(enum comparison cmp, variant *v1, variant *v2);

static failable_variant expression_function_callable_executor(list *positional_args, dict *named_args, void *callable_data, variant *this_obj, exec_context *ctx);



void initialize_expression_execution() {
    // used for inc/dec operations
    one = new_numeric_literal_expression("1", NULL);
}

failable_variant execute_expression(expression *e, exec_context *ctx) {
    // first concern is whether the expression stores data, or is read only
    expression_type et = expression_get_type(e);
    operator_type op = expression_get_operator(e);
    expression *lval_expr;
    expression *rval_expr;

    if (et == ET_UNARY_OP) {
        lval_expr = expression_get_operand(e, 0);
        switch (op) {
            case OP_PRE_INC:  return modify_and_store(lval_expr, MAS_ADD, one, false, ctx);
            case OP_PRE_DEC:  return modify_and_store(lval_expr, MAS_SUB, one, false, ctx);
            case OP_POST_INC: return modify_and_store(lval_expr, MAS_ADD, one, true, ctx);
            case OP_POST_DEC: return modify_and_store(lval_expr, MAS_SUB, one, true, ctx);
        }

    } else if (et == ET_BINARY_OP) {
        lval_expr = expression_get_operand(e, 0);
        rval_expr = expression_get_operand(e, 1);
        switch (op) {
            case OP_ASSIGNMENT:
                failable_variant retrieval = retrieve_value(rval_expr, ctx, NULL);
                if (retrieval.failed) return failed_variant(&retrieval, NULL);
                failable storage = store_value(lval_expr, ctx, retrieval.result);
                if (storage.failed) return failed_variant(&storage, "Cannot store");
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

static failable_variant retrieve_value(expression *e, exec_context *ctx, variant **this_value) {
    failable_variant execution, variant1, variant2;
    operator_type op = expression_get_operator(e);
    const char *data = expression_get_terminal_data(e);
    expression *operand1, *operand2;

    switch (expression_get_type(e)) {
        case ET_IDENTIFIER:
            variant *v = exec_context_resolve_symbol(ctx, data);
            if (v == NULL || variant_is_null(v))
                return failed_variant(NULL, "identifier not found \"%s\"", data);
            return ok_variant(v);
        case ET_NUMERIC_LITERAL:
            return ok_variant(new_int_variant(atoi(data)));
        case ET_STRING_LITERAL:
            return ok_variant(new_str_variant(data));
        case ET_BOOLEAN_LITERAL:
            return ok_variant(new_bool_variant(strcmp(data, "true") == 0));
        case ET_LIST_DATA:
            list *expressions_list = expression_get_list_data(e);
            list *values_list = new_list(containing_variants);
            for_list(expressions_list, list_iter, expression, list_exp) {
                execution = execute_expression(list_exp, ctx);
                if (execution.failed) return failed_variant(&execution, NULL);
                list_add(values_list, execution.result);
            }
            return ok_variant(new_list_variant(values_list));
        case ET_DICT_DATA:
            dict *expressions_dict = expression_get_dict_data(e);
            dict *values_dict = new_dict(containing_variants);
            iterator *keys_it = dict_keys_iterator(expressions_dict);
            for_iterator(keys_it, str, key) {
                execution = execute_expression(dict_get(expressions_dict, key), ctx);
                if (execution.failed) return failed_variant(&execution, NULL);
                dict_set(values_dict, key, execution.result);
            }
            return ok_variant(new_dict_variant(values_dict));

        case ET_UNARY_OP:
            operand1 = expression_get_operand(e, 0);
            variant1 = execute_expression(operand1, ctx);
            if (variant1.failed) return failed_variant(&variant1, "Single operand execution failed");
            return calculate_unary_operation(op, variant1.result, ctx);

        case ET_BINARY_OP:
            op = expression_get_operator(e);
            operand1 = expression_get_operand(e, 0);
            operand2 = expression_get_operand(e, 1);

            if (op == OP_ARRAY_SUBSCRIPT) {
                variant1 = execute_expression(operand1, ctx);
                if (variant1.failed) return failed_variant(&variant1, "Operand 1 failed");
                if (!variant_is_list(variant1.result)) return failed_variant(NULL, "ARRAY_SUBSCRIPT requires a list as left operand");
                list *l = variant_as_list(variant1.result);
                variant2 = execute_expression(operand2, ctx);
                if (variant2.failed) return failed_variant(&variant2, "Operand 2 failed");
                if (!variant_is_int(variant2.result)) return failed_variant(NULL, "ARRAY_SUBSCRIPT requires a number value as right operand");
                int index = variant_as_int(variant2.result);
                if (index >= list_length(l)) return failed_variant(NULL, "array index (%d) more than max index (%d)", index, list_length(l) - 1);
                return ok_variant(list_get(l, index));

            } else if (op == OP_MEMBER) {
                return retrieve_member(operand1, operand2, ctx, this_value);

            } else if (op == OP_FUNC_CALL) {
                return make_function_call(operand1, operand2, ctx);

            } else {
                variant1 = execute_expression(operand1, ctx);
                if (variant1.failed) return failed_variant(&variant1, "Operand 1 failed");
                variant2 = execute_expression(operand2, ctx);
                if (variant2.failed) return failed_variant(&variant2, "Operand 2 failed");
                return calculate_binary_operation(op, variant1.result, variant2.result, ctx);
            }
        
        case ET_FUNC_DECL:
            // "retrieving" a `function () { ...}` expression 
            // merely creates and returns a callable variant
            return ok_variant(new_callable_variant(new_callable(
                "(user nameless expression function)",
                expression_function_callable_executor,
                e
            )));
    }

    return failed_variant(NULL, "Cannot retrieve value, unknown expr type / operator_type");
}

static failable_variant modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx) {
    failable_variant retrieval;
    variant *original = NULL;
    int original_int = 0;
    int operand_int;
    int result_int;

    // for now we allow variable creation via simple assignment
    retrieval = retrieve_value(lvalue, ctx, NULL);
    if (retrieval.failed) return failed_variant(&retrieval, "Failed retrieving lvalue");
    original = retrieval.result;
    if (!variant_is_int(original))
        return failed_variant(NULL, "Modify-and-store applies only to integers");
    original_int = variant_as_int(original);

    retrieval = retrieve_value(rvalue, ctx, NULL);
    if (retrieval.failed) return failed_variant(&retrieval, "Failed retrieving rvalue");
    variant *operand = retrieval.result;
    if (!variant_is_int(operand))
        return failed_variant(NULL, "Modify-and-store requires integers as operands");
    operand_int = variant_as_int(operand);

    switch (op) {
        case MAS_ADD: result_int = original_int + operand_int; break;
        case MAS_SUB: result_int = original_int - operand_int; break;
        case MAS_MUL: result_int = original_int * operand_int; break;
        case MAS_DIV: 
            if (operand_int == 0)
                return failed_variant(NULL, "division by zero not possible with integers");
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
    failable storing = store_value(lvalue, ctx, result);
    if (storing.failed)
        return failed_variant(&storing, "Error storing");
    return ok_variant(return_original ? original : result);
}

static failable store_value(expression *lvalue, exec_context *ctx, variant *rvalue) {

    expression_type et = expression_get_type(lvalue);
    if (et == ET_IDENTIFIER) {
        const char *name = expression_get_terminal_data(lvalue);
        if (exec_context_symbol_exists(ctx, name))
            exec_context_update_symbol(ctx, name, rvalue);
        else
            exec_context_register_symbol(ctx, name, rvalue);
        return ok();

    } else if (et == ET_BINARY_OP) {
        operator_type op = expression_get_operator(lvalue);
        if (op == OP_ARRAY_SUBSCRIPT) {
            // e.g. "ARRAY_SUBSCRIPT(<list_like_executionable>, <int_like_executionable>)"
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_variant op1_exec = execute_expression(op1, ctx);
            if (op1_exec.failed) return failed(&op1_exec, NULL);
            if (!variant_is_list(op1_exec.result)) return failed(NULL, "Array subscripts apply only to arrays");

            expression *op2 = expression_get_operand(lvalue, 1);
            failable_variant op2_exec = execute_expression(op2, ctx);
            if (op2_exec.failed) return failed(&op2_exec, NULL);
            if (!variant_is_int(op2_exec.result)) return failed(NULL, "only integer results can be used as array indices");

            list *l = variant_as_list(op1_exec.result);
            int i = variant_as_int(op2_exec.result);
            list_set(l, i, rvalue);
            return ok();

        } else if (op == OP_MEMBER) {
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_variant op1_exec = execute_expression(op1, ctx);
            if (op1_exec.failed) return failed(&op1_exec, NULL);
            if (!variant_is_dict(op1_exec.result)) return failed(NULL, "Struct members work only with identifiers");

            expression *op2 = expression_get_operand(lvalue, 1);
            if (expression_get_type(op2) != ET_IDENTIFIER)
                return failed(NULL, "only identifiers can be used as structure members");

            dict *d = variant_as_dict(op1_exec.result);
            const char *key = expression_get_terminal_data(op2);
            dict_set(d, key, rvalue);
            return ok();

        } else {
            return failed(NULL, "operator_type cannot be used as lvalue: %s", operator_type_to_string(op));
        }
        
    } else {
        return failed(NULL, "expression cannot be used as lvalue: %s", expression_to_string(lvalue));
    }
}

static failable_variant calculate_comparison(enum comparison cmp, variant *v1, variant *v2) {
    if (variant_is_int(v1) && variant_is_int(v2)) {
        int i1 = variant_as_int(v1);
        int i2 = variant_as_int(v2);
        switch (cmp) {
            case COMP_GT: return ok_variant(new_bool_variant(i1 >  i2));
            case COMP_GE: return ok_variant(new_bool_variant(i1 >= i2));
            case COMP_LT: return ok_variant(new_bool_variant(i1 <  i2));
            case COMP_LE: return ok_variant(new_bool_variant(i1 <= i2));
            case COMP_EQ: return ok_variant(new_bool_variant(i1 == i2));
            case COMP_NE: return ok_variant(new_bool_variant(i1 != i2));
        }
    } else if (variant_is_float(v1) && variant_is_float(v2)) {
        float f1 = variant_as_float(v1);
        float f2 = variant_as_float(v2);
        switch (cmp) {
            case COMP_GT: return ok_variant(new_bool_variant(f1 >  f2));
            case COMP_GE: return ok_variant(new_bool_variant(f1 >= f2));
            case COMP_LT: return ok_variant(new_bool_variant(f1 <  f2));
            case COMP_LE: return ok_variant(new_bool_variant(f1 <= f2));
            case COMP_EQ: return ok_variant(new_bool_variant(f1 == f2));
            case COMP_NE: return ok_variant(new_bool_variant(f1 != f2));
        }
    } else if (variant_is_str(v1) && variant_is_str(v2)) {
        int c = strcmp(variant_as_str(v1), variant_as_str(v2));
        switch (cmp) {
            case COMP_GT: return ok_variant(new_bool_variant(c >  0));
            case COMP_GE: return ok_variant(new_bool_variant(c >= 0));
            case COMP_LT: return ok_variant(new_bool_variant(c <  0));
            case COMP_LE: return ok_variant(new_bool_variant(c <= 0));
            case COMP_EQ: return ok_variant(new_bool_variant(c == 0));
            case COMP_NE: return ok_variant(new_bool_variant(c != 0));
        }
    } else if (variant_is_bool(v1) && variant_is_bool(v2)) {
        bool b1 = variant_as_bool(v1);
        bool b2 = variant_as_bool(v2);
        switch (cmp) {
            case COMP_EQ: return ok_variant(new_bool_variant(b1 == b2));
            case COMP_NE: return ok_variant(new_bool_variant(b1 != b2));
        }
    } else if (variant_is_list(v1) && variant_is_list(v2)) {
        list *l1 = variant_as_list(v1);
        list *l2 = variant_as_list(v2);
        bool eq = lists_are_equal(l1, l2);
        switch (cmp) {
            case COMP_EQ: return ok_variant(new_bool_variant(eq));
            case COMP_NE: return ok_variant(new_bool_variant(!eq));
        }
    }

    return failed_variant(NULL, "cannot compare with given operands (%s and %s)", variant_to_string(v1), variant_to_string(v2));
}

static failable_variant retrieve_member(expression *object, expression *member, exec_context *ctx, variant **this_value) {
    /* some objects, like a list, may behave as a dict,
    in the sense of methods, e.g. "list1.length()", or "items.add(item)"
    */
    failable_variant obj_eval = execute_expression(object, ctx);
    if (obj_eval.failed) return failed_variant(&obj_eval, NULL);
    variant *obj = obj_eval.result;
    if (this_value != NULL)
        *this_value = obj;
    
    if (expression_get_type(member) != ET_IDENTIFIER)
        return failed_variant(NULL, "MEMBER_OF requires identifier as right operand");
    const char *member_name = expression_get_terminal_data(member);
    
    dict *d = NULL;
    if (variant_is_dict(obj)) {
        // find member in normal members, fallback to built-it methods
        d = variant_as_dict(obj);
        if (dict_has(d, member_name))
            return ok_variant(dict_get(d, member_name));
        
        d = get_built_in_dict_methods_dictionary();
        if (dict_has(d, member_name))
            return ok_variant(dict_get(d, member_name));

    } else if (variant_is_list(obj)) {
        d = get_built_in_list_methods_dictionary();
        if (dict_has(d, member_name))
            return ok_variant(dict_get(d, member_name));
        
    } else if (variant_is_str(obj)) {
        d = get_built_in_str_methods_dictionary();
        if (dict_has(d, member_name))
            return ok_variant(dict_get(d, member_name));
    }
                    
    return failed_variant(NULL, "member '%s' not found in object", member_name);
}

static failable_variant make_function_call(expression *callable_expr, expression *args_expr, exec_context *ctx) {

    variant *this_value = NULL;
    failable_variant callable_retrieval = retrieve_value(callable_expr, ctx, &this_value);
    if (callable_retrieval.failed) return failed_variant(&callable_retrieval, NULL);
    variant *v = callable_retrieval.result;

    if (v == NULL) return failed_variant(NULL, "could not resolve call target");
    if (!variant_is_callable(v))
        return failed_variant(NULL, "call target is not a callable");
    callable *c = variant_as_callable(v);

    failable_variant args_retrieval = retrieve_value(args_expr, ctx, NULL);
    if (args_retrieval.failed) return failed_variant(&args_retrieval, "error retrieving arguments");
    list *arg_values = variant_as_list(args_retrieval.result);

    return callable_call(c, arg_values, NULL, this_value, ctx);
}

static failable_variant calculate_unary_operation(operator_type op, variant *value, exec_context *ctx) {
    switch (op) {
        case OP_POSITIVE_NUM:
            if (variant_is_int(value) || variant_is_float(value))
                return ok_variant(value);
            return failed_variant(NULL, "positive num only works for int / float values");

        case OP_NEGATIVE_NUM:
            if (variant_is_int(value))
                return ok_variant(new_int_variant(variant_as_int(value) * -1));
            if (variant_is_float(value))
                return ok_variant(new_float_variant(variant_as_float(value) * -1));
            return failed_variant(NULL, "negative num only works for int / float values");

        case OP_LOGICAL_NOT:
            // let's avoid implicit conversion to bool for now.
            if (variant_is_bool(value))
                return ok_variant(new_bool_variant(!variant_as_bool(value)));
            return failed_variant(NULL, "logical not only works for bool values");

        case OP_BITWISE_NOT:
            if (variant_is_int(value))
                return ok_variant(new_int_variant(~variant_as_int(value)));
            return failed_variant(NULL, "bitwise not only works for int values");
    }
    return failed_variant(NULL, "Unknown unary operator_type %s", operator_type_to_string(op));
}

static failable_variant calculate_binary_operation(operator_type op, variant *v1, variant *v2, exec_context *ctx) {
    switch (op) {
        case OP_MULTIPLY:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) * variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) * variant_as_float(v2)));
            if (variant_is_str(v1) && variant_is_int(v2)) {
                str_builder *tmp = new_str_builder();
                for (int i = 0; i < variant_as_int(v2); i++)
                    str_builder_add(tmp, variant_as_str(v1));
                return ok_variant(new_str_variant(str_builder_charptr(tmp)));
            }
            return failed_variant(NULL, "multiplication is only supported in int/float types");

        case OP_DIVIDE:
            if (variant_is_int(v1)) {
                int denominator = variant_as_int(v2);
                if (denominator == 0) return failed_variant(NULL, "division by zero not possible in integers");
                return ok_variant(new_int_variant(variant_as_int(v1) / denominator));
            }
            if (variant_is_float(v1)) {
                // in floats, the result is "infinity"
                return ok_variant(new_float_variant(variant_as_float(v1) / variant_as_float(v2)));
            }
            return failed_variant(NULL, "division is only supported in int/float types");

        case OP_MODULO:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) % variant_as_int(v2)));
            return failed_variant(NULL, "modulo is only supported in int types");

        case OP_ADD:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) + variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) + variant_as_float(v2)));
            if (variant_is_str(v1)) {
                str_builder *sb = new_str_builder();
                str_builder_add(sb, variant_as_str(v1));
                str_builder_add(sb, variant_as_str(v2));
                return ok_variant(new_str_variant(str_builder_charptr(sb)));
            }
            return failed_variant(NULL, "addition is only supported in int, float, string types");

        case OP_SUBTRACT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) - variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) - variant_as_float(v2)));
            return failed_variant(NULL, "subtraction is only supported in int/float types");

        case OP_LSHIFT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) << variant_as_int(v2)));
            return failed_variant(NULL, "left shift is only supported in int types");

        case OP_RSHIFT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) >> variant_as_int(v2)));
            return failed_variant(NULL, "right shift is only supported in int types");

        case OP_LESS_THAN:      return calculate_comparison(COMP_LT, v1, v2);
        case OP_LESS_EQUAL:     return calculate_comparison(COMP_LE, v1, v2);
        case OP_GREATER_THAN:   return calculate_comparison(COMP_GT, v1, v2);
        case OP_GREATER_EQUAL:  return calculate_comparison(COMP_GE, v1, v2);
        case OP_EQUAL:          return calculate_comparison(COMP_EQ, v1, v2);
        case OP_NOT_EQUAL:      return calculate_comparison(COMP_NE, v1, v2);

        case OP_BITWISE_AND:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) & variant_as_int(v2)));
            return failed_variant(NULL, "bitwise operations only supported in int types");
        case OP_BITWISE_XOR:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) ^ variant_as_int(v2)));
            return failed_variant(NULL, "bitwise operations only supported in int types");
        case OP_BITWISE_OR:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) | variant_as_int(v2)));
            return failed_variant(NULL, "bitwise operations only supported in int types");

        case OP_LOGICAL_AND:
            // we could do shorthand here...
            if (variant_is_bool(v1) && variant_is_bool(v2))
                return ok_variant(new_bool_variant(variant_as_bool(v1) && variant_as_bool(v2)));
            return failed_variant(NULL, "logical operations only supported in bool types");
        case OP_LOGICAL_OR:
            // we could do shorthand here...
            if (variant_is_bool(v1) && variant_is_bool(v2))
                return ok_variant(new_bool_variant(variant_as_bool(v1) || variant_as_bool(v2)));
            return failed_variant(NULL, "logical operations only supported in bool types");

        case OP_SHORT_IF:
            if (!variant_is_bool(v1))
                return failed_variant(NULL, "? operator_type requires boolean condition");
            if (!variant_is_list(v2))
                return failed_variant(NULL, "? operator_type was expecting a list of 2 arguments");
            bool passed = variant_as_bool(v1);
            list *values_pair = variant_as_list(v2);
            if (list_length(values_pair) != 2)
                return failed_variant(NULL, "? operator_type was expecting exactly two arguments in the list");
            return ok_variant(list_get(values_pair, passed ? 0 : 1));
    }

    return failed_variant(NULL, "Unknown binary operator_type %s", operator_type_to_string(op));
}

static failable_variant expression_function_callable_executor(
    list *positional_args, 
    dict *named_args, 
    void *callable_data, 
    variant *this_obj,
    exec_context *ctx
) {
    expression *expr = (expression *)callable_data;

    list *arg_names = expression_get_func_arg_names(expr);
    if (list_length(positional_args) < list_length(arg_names))
        return failed_variant(NULL, "expected %d arguments, got only %d", list_length(arg_names), list_length(positional_args));

    stack_frame *f = new_stack_frame("expr_func");
    stack_frame_initialization(f, arg_names, positional_args, named_args, this_obj);
    exec_context_push_stack_frame(ctx, f);

    failable_variant result = execute_statements(expression_get_func_statements(expr), ctx);
    
    exec_context_pop_stack_frame(ctx);

    return result;
}
