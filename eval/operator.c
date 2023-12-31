#include "operator.h"
#include "token.h"

struct op_info {
    enum op_position position;
    token_type token;
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
        if (op_infos[i].token == type && op_infos[i].position == position)
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
    return 999;
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

