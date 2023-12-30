#include "operator.h"
#include "token.h"

enum fix_type {
    PREFIX, POSTFIX, INFIX, NOFIX
};

struct op_info {
    token_type token;
    operator op;
    int operands_count;
    enum fix_type fix;
};

// first entries more precedent than later entries
struct op_info op_infos[] = {
    { T_IDENTIFIER,       OP_SYMBOL_VALUE,   0, NOFIX },
    { T_NUMBER_LITERAL,   OP_NUMBER_VALUE,   0, NOFIX },
 
    { T_PLUS,             OP_ADDITION,       2, INFIX },
    { T_MINUS,            OP_SUBTRACTION,    2, INFIX },
    { T_ASTERISK,         OP_MULTIPLICATION, 2, INFIX },
    { T_FWD_SLASH,        OP_DIVISION,       2, INFIX },

    { T_AMPERSAND,        OP_BINARY_AND,     2, INFIX },
    { T_PIPE,             OP_BINARY_OR,      2, INFIX },
    { T_TIDLE,            OP_BINARY_NOT,     1, PREFIX },

    { T_DOUBLE_AMPERSAND, OP_LOGICAL_AND,    2, INFIX },
    { T_DOUBLE_PIPE,      OP_LOGICAL_OR,     2, INFIX },
    { T_EXCLAMATION,      OP_LOGICAL_NOT,    1, PREFIX },
};

int operator_precedence(operator op) {
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].op == op)
            return i;
    }
    return 999;
}

int operator_operands_count(operator op) {
    for (int i = 0; i < (sizeof(op_infos)/sizeof(op_infos[0])); i++) {
        if (op_infos[i].op == op)
            return op_infos[i].operands_count;
    }
    return 0;
}
