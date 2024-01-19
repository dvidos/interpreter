#ifndef _OPERATOR_H
#define _OPERATOR_H

#include "token.h"


enum op_position {
    POS_UNKNOWN,
    PREFIX,
    POSTFIX,
    INFIX,
};

enum operator {
    OP_UNKNOWN,

    OP_FUNC_CALL,          // a()
    OP_ARRAY_SUBSCRIPT,    // a[b]
    OP_STRUCT_MEMBER_PTR,  // a->b
    OP_STRUCT_MEMBER_REF,  // a.b
    OP_POST_INC,           // a++
    OP_POST_DEC,           // a--
    OP_POSITIVE_NUM,       // +123
    OP_NEGATIVE_NUM,       // -123
    OP_LOGICAL_NOT,        // !a
    OP_BITWISE_NOT,        // ~a
    OP_PRE_INC,            // ++a
    OP_PRE_DEC,            // --a
    OP_POINTED_VALUE,      // *a
    OP_ADDRESS_OF,         // &a
    OP_MULTIPLY,           // *
    OP_DIVIDE,             // /
    OP_MODULO,             // %
    OP_ADD,                // +
    OP_SUBTRACT,           // -
    OP_LSHIFT,             // <<
    OP_RSHIFT,             // >>
    OP_LESS_THAN,          // <
    OP_LESS_EQUAL,         // <=
    OP_GREATER_THAN,       // >
    OP_GREATER_EQUAL,      // >=
    OP_EQUAL,              // ==
    OP_NOT_EQUAL,          // !=
    OP_BITWISE_AND,        // &
    OP_BITWISE_OR,         // |
    OP_BITWISE_XOR,        // ^
    OP_LOGICAL_AND,        // &&
    OP_LOGICAL_OR,         // ||
    OP_SHORT_IF,        // a ? b : c
    OP_ASSIGNMENT,         // =
    OP_ADD_ASSIGN,         // +=
    OP_SUB_ASSIGN,         // -=
    OP_MUL_ASSIGN,         // *=
    OP_DIV_ASSIGN,         // /=
    OP_MOD_ASSIGN,         // %=
    OP_RSH_ASSIGN,         // >>=
    OP_LSH_ASSIGN,         // <<=
    OP_AND_ASSIGN,         // &=
    OP_OR_ASSIGN,          // |=
    OP_XOR_ASSIGN,         // ^=

    OP_SENTINEL, // lowest priority
    OP_MAX_VALUE // to discover the size of the enum
};

typedef enum operator operator;
typedef enum op_position op_position;


void initialize_operator_tables();
operator get_operator_by_token_type_and_position(token_type type, enum op_position position);
int operator_precedence(operator op);
op_position operator_position(operator op);
const char *operator_str(operator op);

extern contained_item *containing_operators;

#endif
