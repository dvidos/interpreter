#include <stdlib.h>
#include <string.h>
#include "execution.h"
#include "../utils/strbuff.h"

// used for pre/post increment/decrement
static expression *one = NULL;

enum modify_and_store {
    MAS_NO_MOD,
    MAS_ADD, MAS_SUB, MAS_MUL, MAS_DIV, MAS_MOD,
    MAS_RSH, MAS_LSH, MAS_AND, MAS_OR, MAS_XOR,
};
enum comparison { 
    COMP_GT, COMP_GE, 
    COMP_LT, COMP_LE, 
    COMP_EQ, COMP_NE
};

static failable_value modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, dict *values);
static failable_value retrieve_value(expression *e, dict *values);
static failable store_value(expression *lvalue, dict *values, value *rvalue);

static failable_value calculate_unary_operation(operator op, value *value, dict *values);
static failable_value calculate_binary_operation(operator op, value *v1, value *v2, dict *values);
static failable_value calculate_ternary_operation(operator op, value *v1, value *v2, value *v3, dict *values);
static failable_value calculate_comparison(enum comparison cmp, value *v1, value *v2);


failable_value execute_expression(expression *e, dict *values) {
    // first concern is whether the expression stores data, or is read only
    expression_type et = expression_get_type(e);
    operator op = expression_get_operator(e);
    expression *lvalue;
    expression *rvalue;

    if (et == ET_UNARY_OP) {
        if (op == OP_PRE_INC || op == OP_PRE_DEC || op == OP_POST_INC || op == OP_POST_DEC) {
            lvalue = expression_get_operand(e, 0);
            bool is_inc = (op == OP_PRE_INC || op == OP_POST_INC);
            bool is_post = (op == OP_POST_INC || op == OP_POST_DEC);
            if (one == NULL)
                one = new_numeric_literal_expression("1");
            return modify_and_store(lvalue, is_inc ? MAS_ADD : MAS_SUB, one, is_post, values);
        } else {
            // all other unary operators are not storing values
            return retrieve_value(e, values);
        }

    } else if (et == ET_BINARY_OP) {
        lvalue = expression_get_operand(e, 0);
        rvalue = expression_get_operand(e, 1);
        switch (op) {
            case OP_ASSIGNMENT: return modify_and_store(lvalue, MAS_NO_MOD, rvalue, false, values);
            case OP_ADD_ASSIGN: return modify_and_store(lvalue, MAS_ADD, rvalue, false, values);
            case OP_SUB_ASSIGN: return modify_and_store(lvalue, MAS_SUB, rvalue, false, values);
            case OP_MUL_ASSIGN: return modify_and_store(lvalue, MAS_MUL, rvalue, false, values);
            case OP_DIV_ASSIGN: return modify_and_store(lvalue, MAS_DIV, rvalue, false, values);
            case OP_MOD_ASSIGN: return modify_and_store(lvalue, MAS_MOD, rvalue, false, values);
            case OP_RSH_ASSIGN: return modify_and_store(lvalue, MAS_RSH, rvalue, false, values);
            case OP_LSH_ASSIGN: return modify_and_store(lvalue, MAS_LSH, rvalue, false, values);
            case OP_AND_ASSIGN: return modify_and_store(lvalue, MAS_AND, rvalue, false, values);
            case OP_OR_ASSIGN:  return modify_and_store(lvalue, MAS_OR,  rvalue, false, values);
            case OP_XOR_ASSIGN: return modify_and_store(lvalue, MAS_XOR, rvalue, false, values);
            default:
                // all other binary operators are not storing values
                return retrieve_value(e, values);
        }

    } else {
        // all other expression types are not storing values
        return retrieve_value(e, values);
    }
}

static failable_value modify_and_store(expression *lvalue, enum modify_and_store op, expression *rvalue, bool return_original, dict *values) {

    failable_value retrieval = retrieve_value(lvalue, values);
    if (retrieval.failed) return failed_value("Failed retrieving lvalue: %s", retrieval.err_msg);
    value *original = retrieval.result;
    if (!value_is_int(original))
        return failed_value("Modify-and-store applies only to integers");

    retrieval = retrieve_value(rvalue, values);
    if (retrieval.failed) return failed_value("Failed retrieving rvalue: %s", retrieval.err_msg);
    value *operand = retrieval.result;
    if (!value_is_int(operand))
        return failed_value("Modify-and-store requires integers as operands");
    
    int a = value_as_int(original);
    int b = value_as_int(operand);
    int c;
    switch (op) {
        case MAS_NO_MOD: c = a; break;
        case MAS_ADD: c = a + b; break;
        case MAS_SUB: c = a - b; break;
        case MAS_MUL: c = a * b; break;
        case MAS_DIV: c = a / b; break;
        case MAS_MOD: c = a % b; break;
        case MAS_RSH: c = a >> b; break;
        case MAS_LSH: c = a << b; break;
        case MAS_AND: c = a & b; break;
        case MAS_OR:  c = a | b; break;
        case MAS_XOR: c = a ^ b; break;
    }

    value *result = new_int_value(c);
    failable storing = store_value(lvalue, values, result);
    if (storing.failed)
        return failed_value("Error storing: %s", storing.err_msg);
    return ok_value(return_original ? original : result);
}

static failable_value retrieve_value(expression *e, dict *values) {
    failable_value execution, v1, v2, v3;
    operator op = expression_get_operator(e);
    const char *td = expression_get_terminal_data(e);
    expression *op1, *op2, *op3;

    switch (expression_get_type(e)) {
        case ET_IDENTIFIER:
            value *v = dict_get(values, td);
            if (v == NULL || value_is_null(v))
                return failed_value("identifier not found \"%s\"", td);
            return ok_value(v);
        case ET_NUMERIC_LITERAL:
            return ok_value(new_int_value(atoi(td)));
        case ET_STRING_LITERAL:
            return ok_value(new_str_value(td));
        case ET_BOOLEAN_LITERAL:
            return ok_value(new_bool_value(strcmp(td, "true") == 0));
        case ET_FUNC_ARGS:
            list *arg_expressions = expression_get_func_args(e);
            list *arg_values = new_list();
            for_list(arg_expressions, args_iterator, expression, arg_exp) {
                execution = execute_expression(arg_exp, values);
                if (execution.failed) return failed_value("%s", execution.err_msg);
                list_add(arg_values, execution.result);
            }
            return ok_value(new_list_value(arg_values));
        
        case ET_UNARY_OP:
            op1 = expression_get_operand(e, 0);
            v1 = execute_expression(op1, values);
            if (v1.failed) return failed_value("%s", v1);
            return calculate_unary_operation(op, v1.result, values);

        case ET_BINARY_OP:
            op1 = expression_get_operand(e, 0);
            v1 = execute_expression(op1, values);
            if (v1.failed) return failed_value("%s", v1.err_msg);
            op2 = expression_get_operand(e, 1);
            v2 = execute_expression(op2, values);
            if (v2.failed) return failed_value("%s", v2.err_msg);
            return calculate_binary_operation(op, v1.result, v2.result, values);

        case ET_TERNARY_OP:
            switch (op) {
                case OP_CONDITIONAL:
                    op1 = expression_get_operand(e, 0);
                    v1 = execute_expression(op1, values);
                    if (v1.failed) return failed_value("%s", v1.err_msg);
                    if (v1.result) {
                        op2 = expression_get_operand(e, 1);
                        return execute_expression(op2, values);
                    } else {
                        op3 = expression_get_operand(e, 2);
                        return execute_expression(op3, values);
                    }
            }
    }

    return failed_value("Cannot retrieve value, unknown expr type / operator");
}

static failable store_value(expression *lvalue, dict *values, value *rvalue) {
    // example: "i" (IDENTIFIER("i"))
    // example lvalue: "persons[i+1].age"
    // STRUCT_MEMBER(ARRAY_ITEM(IDENT("persons"), ADD(IDENT("i"), NUM("1")), IDENT("age"))
    // other: "persons[i].children[j].cousings[k].games"
    // MEMBER(ITEM(MEMBER(ITEM(MEMBER(ITEM("persons", "i"), "children"), "j"), "cousins"), "k"), "games")

    expression_type et = expression_get_type(lvalue);
    if (et == ET_IDENTIFIER) {
        dict_set(values, expression_get_terminal_data(lvalue), rvalue);
        return succeeded();

    } else if (et == ET_BINARY_OP) {
        operator op = expression_get_operator(lvalue);
        if (op == OP_ARRAY_SUBSCRIPT) {
            // e.g. "ARRAY_SUBSCRIPT(<list_like_executionable>, <int_like_executionable>)"
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_value op1_exec = execute_expression(op1, values);
            if (op1_exec.failed) return failed("%s", op1_exec.err_msg);
            if (!value_is_list(op1_exec.result)) return failed("Array subscripts apply only to arrays");

            expression *op2 = expression_get_operand(lvalue, 1);
            failable_value op2_exec = execute_expression(op2, values);
            if (op2_exec.failed) return failed("%s", op2_exec.err_msg);
            if (!value_is_int(op2_exec.result)) return failed("only integer results can be used as array indices");

            list *l = value_as_list(op1_exec.result);
            int i = value_as_int(op2_exec.result);
            list_set(l, i, rvalue);
            return succeeded();

        } else if (op == OP_STRUCT_MEMBER_REF) {
            // e.g. "ARRAY_SUBSCRIPT(<something_that_evaluates_into_dict>, IDENTIFIER("age"))"
            expression *op1 = expression_get_operand(lvalue, 0);
            failable_value op1_exec = execute_expression(op1, values);
            if (op1_exec.failed) return failed("%s", op1_exec.err_msg);
            if (!value_is_dict(op1_exec.result)) return failed("Struct members work only with identifiers");

            expression *op2 = expression_get_operand(lvalue, 1);
            if (expression_get_type(op2) != ET_IDENTIFIER)
                return failed("only identifiers can be used as structure members");

            dict *d = value_as_dict(op1_exec.result);
            const char *key = expression_get_terminal_data(op2);
            dict_set(d, key, rvalue);
            return succeeded();

        } else {
            return failed("operator cannot be used as lvalue: %s", operator_str(op));
        }
        
    } else {
        return failed("expression cannot be used as lvalue: %s", expression_to_string(lvalue));
    }
}

failable_value calculate_comparison(enum comparison cmp, value *v1, value *v2) {
    if (value_is_int(v1) && value_is_int(v2)) {
        int i1 = value_as_int(v1);
        int i2 = value_as_int(v2);
        switch (cmp) {
            case COMP_GT: return ok_value(new_bool_value(i1 >  i2));
            case COMP_GE: return ok_value(new_bool_value(i1 >= i2));
            case COMP_LT: return ok_value(new_bool_value(i1 <  i2));
            case COMP_LE: return ok_value(new_bool_value(i1 <= i2));
            case COMP_EQ: return ok_value(new_bool_value(i1 == i2));
            case COMP_NE: return ok_value(new_bool_value(i1 != i2));
        }
    } else if (value_is_float(v1) && value_is_float(v2)) {
        float f1 = value_as_float(v1);
        float f2 = value_as_float(v2);
        switch (cmp) {
            case COMP_GT: return ok_value(new_bool_value(f1 >  f2));
            case COMP_GE: return ok_value(new_bool_value(f1 >= f2));
            case COMP_LT: return ok_value(new_bool_value(f1 <  f2));
            case COMP_LE: return ok_value(new_bool_value(f1 <= f2));
            case COMP_EQ: return ok_value(new_bool_value(f1 == f2));
            case COMP_NE: return ok_value(new_bool_value(f1 != f2));
        }
    } else if (value_is_str(v1) && value_is_str(v2)) {
        int c = strcmp(value_as_str(v1), value_as_str(v2));
        switch (cmp) {
            case COMP_GT: return ok_value(new_bool_value(c >  0));
            case COMP_GE: return ok_value(new_bool_value(c >= 0));
            case COMP_LT: return ok_value(new_bool_value(c <  0));
            case COMP_LE: return ok_value(new_bool_value(c <= 0));
            case COMP_EQ: return ok_value(new_bool_value(c == 0));
            case COMP_NE: return ok_value(new_bool_value(c != 0));
        }
    } else if (value_is_bool(v1) && value_is_bool(v2)) {
        bool b1 = value_as_bool(v1);
        bool b2 = value_as_bool(v2);
        switch (cmp) {
            case COMP_EQ: return ok_value(new_bool_value(b1 == b2));
            case COMP_NE: return ok_value(new_bool_value(b1 != b2));
        }
    } else if (value_is_list(v1) && value_is_list(v2)) {
        list *l1 = value_as_list(v1);
        list *l2 = value_as_list(v2);
        bool eq = lists_are_equal(l1, l2);
        switch (cmp) {
            case COMP_EQ: return ok_value(new_bool_value(eq));
            case COMP_NE: return ok_value(new_bool_value(!eq));
        }
    }

    return failed_value("cannot compare with given operands (%s and %s)", value_to_string(v1), value_to_string(v2));
}


static failable_value calculate_unary_operation(operator op, value *value, dict *values) {
    switch (op) {
        case OP_POSITIVE_NUM:
            if (value_is_int(value) || value_is_float(value))
                return ok_value(value);
            return failed_value("positive num only works for int / float values");

        case OP_NEGATIVE_NUM:
            if (value_is_int(value))
                return ok_value(new_int_value(value_as_int(value) * -1));
            if (value_is_float(value))
                return ok_value(new_float_value(value_as_float(value) * -1));
            return failed_value("negative num only works for int / float values");

        case OP_LOGICAL_NOT:
            // let's avoid implicit conversion to bool for now.
            if (value_is_bool(value))
                return ok_value(new_bool_value(!value_as_bool(value)));
            return failed_value("logical not only works for bool values");

        case OP_BITWISE_NOT:
            if (value_is_int(value))
                return ok_value(new_int_value(~value_as_int(value)));
            return failed_value("bitwise not only works for int values");

        // case OP_POINTED_VALUE:
        // case OP_ADDRESS_OF:
    }
    return failed_value("Unknown unary operator %s", operator_str(op));
}

static failable_value calculate_binary_operation(operator op, value *v1, value *v2, dict *values) {
    switch (op) {
        case OP_FUNC_CALL:
            return failed_value("function calls not supported yet!");

        case OP_MULTIPLY:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) * value_as_int(v2)));
            if (value_is_float(v1))
                return ok_value(new_float_value(value_as_float(v1) * value_as_float(v2)));
            return failed_value("multiplication is only supported in int/float types");

        case OP_DIVIDE:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) / value_as_int(v2)));
            if (value_is_float(v1))
                return ok_value(new_float_value(value_as_float(v1) / value_as_float(v2)));
            return failed_value("division is only supported in int/float types");

        case OP_MODULO:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) % value_as_int(v2)));
            return failed_value("modulo is only supported in int types");

        case OP_ADD:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) + value_as_int(v2)));
            if (value_is_float(v1))
                return ok_value(new_float_value(value_as_float(v1) + value_as_float(v2)));
            if (value_is_str(v1)) {
                strbuff *sb = new_strbuff();
                strbuff_cat(sb, value_as_str(v1));
                strbuff_cat(sb, value_as_str(v2));
                return ok_value(new_str_value(strbuff_charptr(sb)));
            }
            return failed_value("addition is only supported in int, float, string types");

        case OP_SUBTRACT:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) - value_as_int(v2)));
            if (value_is_float(v1))
                return ok_value(new_float_value(value_as_float(v1) - value_as_float(v2)));
            return failed_value("subtraction is only supported in int/float types");

        case OP_LSHIFT:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) << value_as_int(v2)));
            return failed_value("left shift is only supported in int types");

        case OP_RSHIFT:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) >> value_as_int(v2)));
            return failed_value("right shift is only supported in int types");

        case OP_LESS_THAN:      return calculate_comparison(COMP_LT, v1, v2);
        case OP_LESS_EQUAL:     return calculate_comparison(COMP_LE, v1, v2);
        case OP_GREATER_THAN:   return calculate_comparison(COMP_GT, v1, v2);
        case OP_GREATER_EQUAL:  return calculate_comparison(COMP_GE, v1, v2);
        case OP_EQUAL:          return calculate_comparison(COMP_EQ, v1, v2);
        case OP_NOT_EQUAL:      return calculate_comparison(COMP_NE, v1, v2);

        case OP_BITWISE_AND:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) & value_as_int(v2)));
            return failed_value("bitwise operations only supported in int types");
        case OP_BITWISE_XOR:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) ^ value_as_int(v2)));
            return failed_value("bitwise operations only supported in int types");
        case OP_BITWISE_OR:
            if (value_is_int(v1))
                return ok_value(new_int_value(value_as_int(v1) | value_as_int(v2)));
            return failed_value("bitwise operations only supported in int types");

        case OP_LOGICAL_AND:
            // we could do shorthand here...
            if (value_is_bool(v1) && value_is_bool(v2))
                return ok_value(new_bool_value(value_as_bool(v1) && value_as_bool(v2)));
            return failed_value("logical operations only supported in bool types");
        case OP_LOGICAL_OR:
            // we could do shorthand here...
            if (value_is_bool(v1) && value_is_bool(v2))
                return ok_value(new_bool_value(value_as_bool(v1) || value_as_bool(v2)));
            return failed_value("logical operations only supported in bool types");
    }

    return failed_value("Unknown binary operator %s", operator_str(op));
}

static failable_value calculate_ternary_operation(operator op, value *v1, value *v2, value *v3, dict *values) {
    switch (op) {
        case OP_CONDITIONAL:
            return ok_value(value_as_bool(v1) ? v2 : v3);
    }

    return failed_value("Unknown ternary operator %s", operator_str(op));
}
