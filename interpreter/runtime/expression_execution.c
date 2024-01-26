#include <stdlib.h>
#include <string.h>
#include "../../utils/str_builder.h"
#include "expression_execution.h"
#include "statement_execution.h"

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
static failable_variant retrieve_value(expression *e, exec_context *ctx);
static failable         store_value(expression *lvalue, exec_context *ctx, variant *rvalue);
static failable_variant make_function_call(expression *func_expr, expression *args_expr, exec_context *ctx);

static failable_variant calculate_unary_operation(operator op, variant *value, exec_context *ctx);
static failable_variant calculate_binary_operation(operator op, variant *v1, variant *v2, exec_context *ctx);
static failable_variant calculate_comparison(enum comparison cmp, variant *v1, variant *v2);

failable_variant function_expression_callable_handler(list *positional_args, dict *named_args, expression *expr, exec_context *ctx);






failable_variant execute_expression(expression *e, exec_context *ctx) {
    // first concern is whether the expression stores data, or is read only
    expression_type et = expression_get_type(e);
    operator op = expression_get_operator(e);
    expression *lval_expr;
    expression *rval_expr;

    if (et == ET_UNARY_OP) {
        if (op == OP_PRE_INC || op == OP_PRE_DEC || op == OP_POST_INC || op == OP_POST_DEC) {
            lval_expr = expression_get_operand(e, 0);
            bool is_inc = (op == OP_PRE_INC || op == OP_POST_INC);
            bool is_post = (op == OP_POST_INC || op == OP_POST_DEC);
            if (one == NULL)
                one = new_numeric_literal_expression("1");
            return modify_and_store(lval_expr, is_inc ? MAS_ADD : MAS_SUB, one, is_post, ctx);
        } else {
            // all other unary operators are not storing values
            return retrieve_value(e, ctx);
        }

    } else if (et == ET_BINARY_OP) {
        lval_expr = expression_get_operand(e, 0);
        rval_expr = expression_get_operand(e, 1);
        switch (op) {
            case OP_ASSIGNMENT:
                failable_variant retrieval = retrieve_value(rval_expr, ctx);
                if (retrieval.failed) return failed_variant("%s", retrieval.err_msg);
                failable storage = store_value(lval_expr, ctx, retrieval.result);
                if (storage.failed) return failed_variant("Cannot store: %s", storage.err_msg);
                return retrieval;
            
            case OP_FUNC_CALL:
                return make_function_call(lval_expr, rval_expr, ctx);
            
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

            default:
                // all other binary operators just retrieve values
                return retrieve_value(e, ctx);
        }

    } else {
        // all other expression types are not storing values
        return retrieve_value(e, ctx);
    }
}

static failable_variant retrieve_value(expression *e, exec_context *ctx) {
    failable_variant execution, variant1, variant2;
    operator op = expression_get_operator(e);
    const char *data = expression_get_terminal_data(e);
    expression *operand1, *operand2;

    switch (expression_get_type(e)) {
        case ET_IDENTIFIER:
            variant *v = resolve_symbol(ctx->symbols, data);
            if (v == NULL || variant_is_null(v))
                return failed_variant("identifier not found \"%s\"", data);
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
                if (execution.failed) return failed_variant("%s", execution.err_msg);
                list_add(values_list, execution.result);
            }
            return ok_variant(new_list_variant(values_list));
        case ET_DICT_DATA:
            dict *expressions_dict = expression_get_dict_data(e);
            dict *values_dict = new_dict(containing_variants, dict_count(expressions_dict));
            iterator *keys_it = dict_keys_iterator(expressions_dict);
            for_iterator(keys_it, str, key) {
                execution = execute_expression(dict_get(expressions_dict, key), ctx);
                if (execution.failed) return failed_variant("%s", execution.err_msg);
                dict_set(values_dict, key, execution.result);
            }
            return ok_variant(new_dict_variant(values_dict));

        case ET_EXPR_PAIR:
            list *values_pair = new_list(containing_variants);
            execution = execute_expression(expression_get_operand(e, 0), ctx);
            if (execution.failed) return failed_variant("%s", execution.err_msg);
            list_add(values_pair, execution.result);
            execution = execute_expression(expression_get_operand(e, 1), ctx);
            if (execution.failed) return failed_variant("%s", execution.err_msg);
            list_add(values_pair, execution.result);
            return ok_variant(new_list_variant(values_pair));

        case ET_UNARY_OP:
            operand1 = expression_get_operand(e, 0);
            variant1 = execute_expression(operand1, ctx);
            if (variant1.failed) return failed_variant("%s", variant1);
            return calculate_unary_operation(op, variant1.result, ctx);

        case ET_BINARY_OP:
            op = expression_get_operator(e);
            operand1 = expression_get_operand(e, 0);
            operand2 = expression_get_operand(e, 1);
            printf("calculating %s (%s, %s)\n", operator_str(op), expression_to_string(operand1), expression_to_string(operand2));
            variant1 = execute_expression(operand1, ctx);
            if (variant1.failed) return failed_variant("Operand 1 failed: %s", variant1.err_msg);

            if (op == OP_ARRAY_SUBSCRIPT) {
                if (!variant_is_list(variant1.result)) return failed_variant("ARRAY_SUBSCRIPT requires a list as left operand");
                list *l = variant_as_list(variant1.result);
                variant2 = execute_expression(operand2, ctx);
                if (variant2.failed) return failed_variant("Operand 2 failed: %s", variant2.err_msg);
                if (!variant_is_int(variant2.result)) return failed_variant("ARRAY_SUBSCRIPT requires a number value as right operand");
                int index = variant_as_int(variant2.result);
                if (index >= list_length(l)) return failed_variant("array index (%d) more than max index (%d)", index, list_length(l) - 1);
                return ok_variant(list_get(l, index));

            } else if (op == OP_MEMBER) {
                if (!variant_is_dict(variant1.result)) return failed_variant("MEMBER_OF requires a dictionary as left operand");
                dict *d = variant_as_dict(variant1.result);
                if (expression_get_type(operand2) != ET_IDENTIFIER) return failed_variant("MEMBER_OF requires identifier as right operand");
                const char *name = expression_get_terminal_data(operand2);
                if (!dict_has(d, name)) return failed_variant("member '%s' not found in dictionary", name);
                return ok_variant(dict_get(d, name));

            } else {
                variant2 = execute_expression(operand2, ctx);
                if (variant2.failed) return failed_variant("Operand 2 failed: %s", variant2.err_msg);
                return calculate_binary_operation(op, variant1.result, variant2.result, ctx);
            }
        
        case ET_FUNC_DECL:
            return ok_variant(new_callable_variant(new_callable(
                "(function expression)",
                "(description)",
                (callable_handler *)function_expression_callable_handler,
                VT_NULL,
                NULL,
                true,
                e
            )));
    }

    return failed_variant("Cannot retrieve value, unknown expr type / operator");
}

static failable_variant modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, exec_context *ctx) {
    failable_variant retrieval;
    variant *original = NULL;
    int original_int = 0;
    int operand_int;
    int result_int;

    // for now we allow variable creation via simple assignment
    retrieval = retrieve_value(lvalue, ctx);
    if (retrieval.failed) return failed_variant("Failed retrieving lvalue: %s", retrieval.err_msg);
    original = retrieval.result;
    if (!variant_is_int(original))
        return failed_variant("Modify-and-store applies only to integers");
    original_int = variant_as_int(original);

    retrieval = retrieve_value(rvalue, ctx);
    if (retrieval.failed) return failed_variant("Failed retrieving rvalue: %s", retrieval.err_msg);
    variant *operand = retrieval.result;
    if (!variant_is_int(operand))
        return failed_variant("Modify-and-store requires integers as operands");
    operand_int = variant_as_int(operand);

    switch (op) {
        case MAS_ADD: result_int = original_int + operand_int; break;
        case MAS_SUB: result_int = original_int - operand_int; break;
        case MAS_MUL: result_int = original_int * operand_int; break;
        case MAS_DIV: 
            if (operand_int == 0)
                return failed_variant("division by zero not possible with integers");
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
        return failed_variant("Error storing: %s", storing.err_msg);
    return ok_variant(return_original ? original : result);
}

static failable store_value(expression *lvalue, exec_context *ctx, variant *rvalue) {
    // example: "i" (IDENTIFIER("i"))
    // example lvalue: "persons[i+1].age"
    // STRUCT_MEMBER(ARRAY_ITEM(IDENT("persons"), ADD(IDENT("i"), NUM("1")), IDENT("age"))
    // other: "persons[i].children[j].cousings[k].games"
    // MEMBER(ITEM(MEMBER(ITEM(MEMBER(ITEM("persons", "i"), "children"), "j"), "cousins"), "k"), "games")

    expression_type et = expression_get_type(lvalue);
    if (et == ET_IDENTIFIER) {
        const char *name = expression_get_terminal_data(lvalue);
        if (!symbol_exists(ctx->symbols, name))
            register_symbol(ctx->symbols, name, rvalue);
        else
            update_symbol(ctx->symbols, name, rvalue);
        return ok();

    } else if (et == ET_BINARY_OP) {
        operator op = expression_get_operator(lvalue);
        if (op == OP_ARRAY_SUBSCRIPT) {
            // e.g. "ARRAY_SUBSCRIPT(<list_like_executionable>, <int_like_executionable>)"
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_variant op1_exec = execute_expression(op1, ctx);
            if (op1_exec.failed) return failed("%s", op1_exec.err_msg);
            if (!variant_is_list(op1_exec.result)) return failed("Array subscripts apply only to arrays");

            expression *op2 = expression_get_operand(lvalue, 1);
            failable_variant op2_exec = execute_expression(op2, ctx);
            if (op2_exec.failed) return failed("%s", op2_exec.err_msg);
            if (!variant_is_int(op2_exec.result)) return failed("only integer results can be used as array indices");

            list *l = variant_as_list(op1_exec.result);
            int i = variant_as_int(op2_exec.result);
            list_set(l, i, rvalue);
            return ok();

        } else if (op == OP_MEMBER) {
            // e.g. "ARRAY_SUBSCRIPT(<something_that_evaluates_into_dict>, IDENTIFIER("age"))"
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_variant op1_exec = execute_expression(op1, ctx);
            if (op1_exec.failed) return failed("%s", op1_exec.err_msg);
            if (!variant_is_dict(op1_exec.result)) return failed("Struct members work only with identifiers");

            expression *op2 = expression_get_operand(lvalue, 1);
            if (expression_get_type(op2) != ET_IDENTIFIER)
                return failed("only identifiers can be used as structure members");

            dict *d = variant_as_dict(op1_exec.result);
            const char *key = expression_get_terminal_data(op2);
            dict_set(d, key, rvalue);
            return ok();

        } else {
            return failed("operator cannot be used as lvalue: %s", operator_str(op));
        }
        
    } else {
        return failed("expression cannot be used as lvalue: %s", expression_to_string(lvalue));
    }
}

failable_variant calculate_comparison(enum comparison cmp, variant *v1, variant *v2) {
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

    return failed_variant("cannot compare with given operands (%s and %s)", variant_to_string(v1), variant_to_string(v2));
}

static failable_variant make_function_call(expression *func_expr, expression *args_expr, exec_context *ctx) {
    // in theory func_name will be an identifier,
    // but in future in might be a reference.

    const char *fname;
    if (expression_get_type(func_expr) != ET_IDENTIFIER)
        return failed_variant("function calls only support identifiers for now");

    fname = expression_get_terminal_data(func_expr);
    variant *v = resolve_symbol(ctx->symbols, fname);
    if (v == NULL)               return failed_variant("function '%s' not found", fname);
    if (!variant_is_callable(v)) return failed_variant("symbol '%s' is not a function", fname);
    callable *c = variant_as_callable(v);

    failable_variant args_list = retrieve_value(args_expr, ctx);
    if (args_list.failed) return failed_variant("error retrieving arguments: %s", args_list.err_msg);

    return callable_call(c, variant_as_list(args_list.result), NULL, ctx);
}


static failable_variant calculate_unary_operation(operator op, variant *value, exec_context *ctx) {
    switch (op) {
        case OP_POSITIVE_NUM:
            if (variant_is_int(value) || variant_is_float(value))
                return ok_variant(value);
            return failed_variant("positive num only works for int / float values");

        case OP_NEGATIVE_NUM:
            if (variant_is_int(value))
                return ok_variant(new_int_variant(variant_as_int(value) * -1));
            if (variant_is_float(value))
                return ok_variant(new_float_variant(variant_as_float(value) * -1));
            return failed_variant("negative num only works for int / float values");

        case OP_LOGICAL_NOT:
            // let's avoid implicit conversion to bool for now.
            if (variant_is_bool(value))
                return ok_variant(new_bool_variant(!variant_as_bool(value)));
            return failed_variant("logical not only works for bool values");

        case OP_BITWISE_NOT:
            if (variant_is_int(value))
                return ok_variant(new_int_variant(~variant_as_int(value)));
            return failed_variant("bitwise not only works for int values");

        // case OP_POINTED_VALUE:
        // case OP_ADDRESS_OF:
    }
    return failed_variant("Unknown unary operator %s", operator_str(op));
}

static failable_variant calculate_binary_operation(operator op, variant *v1, variant *v2, exec_context *ctx) {
    switch (op) {
        case OP_FUNC_CALL:
            // v1 should be a callable, v2 is a list of arguments.
            if (!variant_is_callable(v1)) return failed_variant("function call target is not a callable");
            if (!variant_is_list(v2)) return failed_variant("function call args is not a list");
            callable *c = variant_as_callable(v1);
            list *args = variant_as_list(v2);
            failable_variant execution = callable_call(c, args, NULL, ctx);
            return execution;

        case OP_MULTIPLY:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) * variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) * variant_as_float(v2)));
            if (variant_is_str(v1) && variant_is_int(v2)) {
                str_builder *tmp = new_str_builder();
                for (int i = 0; i < variant_as_int(v2); i++)
                    str_builder_cat(tmp, variant_as_str(v1));
                return ok_variant(new_str_variant(str_builder_charptr(tmp)));
            }
            return failed_variant("multiplication is only supported in int/float types");

        case OP_DIVIDE:
            if (variant_is_int(v1)) {
                int denominator = variant_as_int(v2);
                if (denominator == 0) return failed_variant("division by zero not possible in integers");
                return ok_variant(new_int_variant(variant_as_int(v1) / denominator));
            }
            if (variant_is_float(v1)) {
                // in floats, the result is "infinity"
                return ok_variant(new_float_variant(variant_as_float(v1) / variant_as_float(v2)));
            }
            return failed_variant("division is only supported in int/float types");

        case OP_MODULO:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) % variant_as_int(v2)));
            return failed_variant("modulo is only supported in int types");

        case OP_ADD:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) + variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) + variant_as_float(v2)));
            if (variant_is_str(v1)) {
                str_builder *sb = new_str_builder();
                str_builder_cat(sb, variant_as_str(v1));
                str_builder_cat(sb, variant_as_str(v2));
                return ok_variant(new_str_variant(str_builder_charptr(sb)));
            }
            return failed_variant("addition is only supported in int, float, string types");

        case OP_SUBTRACT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) - variant_as_int(v2)));
            if (variant_is_float(v1))
                return ok_variant(new_float_variant(variant_as_float(v1) - variant_as_float(v2)));
            return failed_variant("subtraction is only supported in int/float types");

        case OP_LSHIFT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) << variant_as_int(v2)));
            return failed_variant("left shift is only supported in int types");

        case OP_RSHIFT:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) >> variant_as_int(v2)));
            return failed_variant("right shift is only supported in int types");

        case OP_LESS_THAN:      return calculate_comparison(COMP_LT, v1, v2);
        case OP_LESS_EQUAL:     return calculate_comparison(COMP_LE, v1, v2);
        case OP_GREATER_THAN:   return calculate_comparison(COMP_GT, v1, v2);
        case OP_GREATER_EQUAL:  return calculate_comparison(COMP_GE, v1, v2);
        case OP_EQUAL:          return calculate_comparison(COMP_EQ, v1, v2);
        case OP_NOT_EQUAL:      return calculate_comparison(COMP_NE, v1, v2);

        case OP_BITWISE_AND:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) & variant_as_int(v2)));
            return failed_variant("bitwise operations only supported in int types");
        case OP_BITWISE_XOR:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) ^ variant_as_int(v2)));
            return failed_variant("bitwise operations only supported in int types");
        case OP_BITWISE_OR:
            if (variant_is_int(v1))
                return ok_variant(new_int_variant(variant_as_int(v1) | variant_as_int(v2)));
            return failed_variant("bitwise operations only supported in int types");

        case OP_LOGICAL_AND:
            // we could do shorthand here...
            if (variant_is_bool(v1) && variant_is_bool(v2))
                return ok_variant(new_bool_variant(variant_as_bool(v1) && variant_as_bool(v2)));
            return failed_variant("logical operations only supported in bool types");
        case OP_LOGICAL_OR:
            // we could do shorthand here...
            if (variant_is_bool(v1) && variant_is_bool(v2))
                return ok_variant(new_bool_variant(variant_as_bool(v1) || variant_as_bool(v2)));
            return failed_variant("logical operations only supported in bool types");

        case OP_SHORT_IF:
            if (!variant_is_bool(v1))
                return failed_variant("? operator requires boolean condition");
            if (!variant_is_list(v2))
                return failed_variant("? operator was expecting a pair (list) of arguments");
            bool passed = variant_as_bool(v1);
            list *values_pair = variant_as_list(v2);
            if (list_length(values_pair) != 2)
                return failed_variant("? operator was expecting exactly two arguments in the list");
            return ok_variant(list_get(values_pair, passed ? 0 : 1));

    }

    return failed_variant("Unknown binary operator %s", operator_str(op));
}


failable_variant function_expression_callable_handler(list *positional_args, dict *named_args, expression *expr, exec_context *ctx) {

    list *arg_names = expression_get_func_arg_names(expr);
    if (list_length(positional_args) != list_length(arg_names))
        return failed_variant("expected %d arguments, got %d", list_length(arg_names), list_length(positional_args));

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
        
    failable_variant result = execute_statements(expression_get_func_statements(expr), ctx);

    ctx->symbols = local_symbols->parent;
    return result;
}

