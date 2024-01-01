#include "operator.h"
#include "token.h"

struct op_info {
    token_type type;
    op_position position;
    operator op;
    int operands_count;
};

// first entries more precedent than later entries
static struct op_info op_infos[] = {
    { T_IDENTIFIER,       OPERAND,   OP_SYMBOL_VALUE,   0 },
    { T_NUMBER_LITERAL,   OPERAND,   OP_NUMBER_VALUE,   0 },
    { T_PLUS,             INFIX,     OP_ADDITION,       2 },
    { T_PLUS,             PREFIX,    OP_POSITIVE_NUM,   1 },
    { T_MINUS,            INFIX,     OP_SUBTRACTION,    2 },
    { T_MINUS,            PREFIX,    OP_NEGATIVE_NUM,   1 },
    { T_ASTERISK,         INFIX,     OP_MULTIPLICATION, 2 },
    { T_FWD_SLASH,        INFIX,     OP_DIVISION,       2 },
    { T_AMPERSAND,        INFIX,     OP_BINARY_AND,     2 },
    { T_PIPE,             INFIX,     OP_BINARY_OR,      2 },
    { T_TIDLE,            PREFIX,    OP_BINARY_NOT,     1 },
    { T_DOUBLE_AMPERSAND, INFIX,     OP_LOGICAL_AND,    2 },
    { T_DOUBLE_PIPE,      INFIX,     OP_LOGICAL_OR,     2 },
    { T_EXCLAMATION,      PREFIX,    OP_LOGICAL_NOT,    1 },
};

operator get_operator_by_token_type_and_position(token_type type, enum op_position position) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // we can convert this to a tree / hashtable later on.
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].type == type && op_infos[i].position == position)
            return op_infos[i].op;
    }
    return OP_UNKNOWN;
}

int operator_precedence(operator op) {
    // we can convert this to a tree / hashtable later on.
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].op == op)
            return i;
    }
    return LOWEST_OPERATOR_PRECEDENCE;
}

op_position operator_position(operator op) {
    // we can convert this to a tree / hashtable later on.
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].op == op)
            return op_infos[i].position;
    }
    return OPERAND;
}

int operator_operands_count(operator op) {
    // we can convert this to a tree / hashtable later on.
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].op == op)
            return op_infos[i].operands_count;
    }
    return 0;
}

const char *operator_str(operator op) {
    switch (op) {
        case OP_UNKNOWN: return "OP_UNKNOWN";
        case OP_SYMBOL_VALUE: return "OP_SYMBOL_VALUE";
        case OP_NUMBER_VALUE: return "OP_NUMBER_VALUE";
        case OP_STRING_VALUE: return "OP_STRING_VALUE";
        case OP_LOGICAL_VALUE: return "OP_LOGICAL_VALUE";
        case OP_ADDITION: return "OP_ADDITION";
        case OP_SUBTRACTION: return "OP_SUBTRACTION";
        case OP_MULTIPLICATION: return "OP_MULTIPLICATION";
        case OP_DIVISION: return "OP_DIVISION";
        case OP_POSITIVE_NUM: return "OP_POSITIVE_NUM";
        case OP_NEGATIVE_NUM: return "OP_NEGATIVE_NUM";
        case OP_LOGICAL_AND: return "OP_LOGICAL_AND";
        case OP_LOGICAL_OR: return "OP_LOGICAL_OR";
        case OP_LOGICAL_NOT: return "OP_LOGICAL_NOT";
        case OP_BINARY_AND: return "OP_BINARY_AND";
        case OP_BINARY_OR: return "OP_BINARY_OR";
        case OP_BINARY_NOT: return "OP_BINARY_NOT";
        case OP_SHORTHAND_IF: return "OP_SHORTHAND_IF";
        case OP_SENTINEL: return "OP_SENTINEL";
        case OP_MAX_VALUE: return "OP_MAX_VALUE";
    }
    return "(unknown)";
}
