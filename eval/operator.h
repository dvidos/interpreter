#ifndef _OPERATOR_H
#define _OPERATOR_H

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

    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,

    OP_BINARY_AND,
    OP_BINARY_OR,
    OP_BINARY_NOT, // unary

    OP_SHORTHAND_IF, // ternary, and actually bouble infix!
    OP_MAX_VALUE // to discover the size of the enum
};

typedef enum operator operator;

int operator_precedence(operator op);
int operator_operands_count(operator op);


#endif
