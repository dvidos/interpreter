#include "execution.h"
#include "../utils/strbuff.h"

failable_value execute_unary_operation(operator op, value *value, dict *values) {
    switch (op) {
        case OP_POST_INC:
        case OP_POST_DEC:
        case OP_PRE_INC:
        case OP_PRE_DEC:
        case OP_POSITIVE_NUM:
        case OP_NEGATIVE_NUM:

        case OP_LOGICAL_NOT:
            return ok_value(new_bool_value(!value_as_bool(value)));
        case OP_BITWISE_NOT:
        case OP_POINTED_VALUE:
        case OP_ADDRESS_OF:
    }

    return failed_value("Unknown unary operator %s", operator_str(op));
}

failable_value execute_binary_operation(operator op, value *v1, value *v2, dict *values) {
    switch (op) {
        case OP_FUNC_CALL:
        case OP_ARRAY_SUBSCRIPT:
        case OP_STRUCT_MEMBER_PTR:
        case OP_STRUCT_MEMBER_REF:
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
        case OP_RSHIFT:
        case OP_LESS_THAN:
        case OP_LESS_EQUAL:
        case OP_GREATER_THAN:
        case OP_GREATER_EQUAL:
        case OP_EQUAL:
        case OP_NOT_EQUAL:
        case OP_BITWISE_AND:
        case OP_BITWISE_XOR:
        case OP_BITWISE_OR:
        case OP_LOGICAL_AND:
        case OP_LOGICAL_OR:
        case OP_ASSIGNMENT:
        case OP_ADD_ASSIGN:
        case OP_SUB_ASSIGN:
        case OP_MUL_ASSIGN:
        case OP_DIV_ASSIGN:
        case OP_MOD_ASSIGN:
        case OP_RSH_ASSIGN:
        case OP_LSH_ASSIGN:
        case OP_AND_ASSIGN:
        case OP_OR_ASSIGN:
        case OP_XOR_ASSIGN:
    }
    return failed_value("Unknown binary operator %s", operator_str(op));
}

failable_value execute_ternary_operation(operator op, value *v1, value *v2, value *v3, dict *values) {
    switch (op) {
        case OP_CONDITIONAL:
            return ok_value(value_as_bool(v1) ? v2 : v3);
    }
    return failed_value("Unknown ternary operator %s", operator_str(op));
}
