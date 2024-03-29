#ifndef _OPERATOR_TYPE_H
#define _OPERATOR_TYPE_H

#include "token.h"


enum op_type_position {
    POS_UNKNOWN,
    PREFIX,
    POSTFIX,
    INFIX,
};

enum op_type_associativity {
    L2R, // aka left-associativity (e.g. subtract)
    R2L  // aka right-associativity (e.g. bitwise xor)
};

enum operator_type {
    OP_UNKNOWN,

    OP_FUNC_CALL,          // a()
    OP_ARRAY_SUBSCRIPT,    // a[b]
  //OP_STRUCT_MEMBER_PTR,  // a->b
    OP_MEMBER,  // a.b
    OP_POST_INC,           // a++
    OP_POST_DEC,           // a--
    OP_POSITIVE_NUM,       // +123
    OP_NEGATIVE_NUM,       // -123
    OP_LOGICAL_NOT,        // !a
    OP_BITWISE_NOT,        // ~a
    OP_PRE_INC,            // ++a
    OP_PRE_DEC,            // --a
  //OP_POINTED_VALUE,      // *a
  //OP_ADDRESS_OF,         // &a
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

typedef enum operator_type operator_type;
typedef enum op_type_position op_type_position;
typedef enum op_type_associativity op_type_associativity;

void initialize_operator_type_tables();
operator_type operator_type_by_token_and_position(token_type type, enum op_type_position position);
int operator_type_precedence(operator_type op);
op_type_position operator_type_position(operator_type op);
op_type_associativity operator_type_associativity(operator_type op);
bool operator_type_is_unary(operator_type op);
const char *operator_type_name(operator_type op);
const void operator_type_describe(operator_type op, str *str);

extern contained_item_info *operator_type_item_info;

#endif
