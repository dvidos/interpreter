#ifndef _OPERATOR_H
#define _OPERATOR_H

#include "token.h"


enum op_position {
    PREFIX,
    POSTFIX,
    INFIX,
    OPERAND
};

enum operator {
    OP_UNKNOWN,

    OP_SYMBOL_VALUE,
    OP_NUMBER_VALUE,
    OP_STRING_VALUE,
    OP_LOGICAL_VALUE,

    OP_ADDITION,
    OP_SUBTRACTION,
    OP_MULTIPLICATION,
    OP_DIVISION,
    OP_POSITIVE_NUM,
    OP_NEGATIVE_NUM,

    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,

    OP_BINARY_AND,
    OP_BINARY_OR,
    OP_BINARY_NOT, // unary

    OP_SHORTHAND_IF, // ternary, and actually bouble infix!

    OP_SENTINEL, // lowest priority
    OP_MAX_VALUE // to discover the size of the enum
};

typedef enum operator operator;
typedef enum op_position op_position;

#define LOWEST_OPERATOR_PRECEDENCE    OP_SENTINEL

operator get_operator_by_token_type_and_position(token_type type, enum op_position position);
int operator_precedence(operator op);
op_position operator_position(operator op);
int operator_operands_count(operator op);
const char *operator_str(operator op);

#endif
