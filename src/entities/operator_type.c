#include <string.h>
#include "operator_type.h"

struct op_type_info {
    operator_type op;
    int priority;
    token_type token_type;
    op_type_position position;
    op_type_associativity associativity;
    const char *name;
};

// first entries more precedent than later entries
static struct op_type_info operators_flat_list[] = {
    { OP_POST_INC,            1, T_DOUBLE_PLUS,       POSTFIX, L2R, "POST_INC" },  // a++
    { OP_POST_DEC,            1, T_DOUBLE_MINUS,      POSTFIX, L2R, "POST_DEC" },  // a--
    { OP_FUNC_CALL,           1, T_LPAREN,            INFIX,   L2R, "CALL" },  // a()
    { OP_ARRAY_SUBSCRIPT,     1, T_LSQBRACKET,        INFIX,   L2R, "ARRAY_SUBSCR" },  // a[b]
  //{ OP_STRUCT_MEMBER_PTR,   1, T_ARROW,             INFIX,   L2R, "STRUCT_MEMBER_PTR" },  // a->b
    { OP_MEMBER,              1, T_DOT,               INFIX,   L2R, "MEMBER" },  // a.b
    { OP_PRE_INC,             2, T_DOUBLE_PLUS,       PREFIX,  R2L, "PRE_INC" },  // ++a
    { OP_PRE_DEC,             2, T_DOUBLE_MINUS,      PREFIX,  R2L, "PRE_DEC" },  // --a
    { OP_POSITIVE_NUM,        2, T_PLUS,              PREFIX,  R2L, "POSITIVE_NUM" },  // +123
    { OP_NEGATIVE_NUM,        2, T_MINUS,             PREFIX,  R2L, "NEGATIVE_NUM" },  // -123
    { OP_LOGICAL_NOT,         2, T_EXCLAMATION,       PREFIX,  R2L, "LOGICAL_NOT" },  // !a
    { OP_BITWISE_NOT,         2, T_TIDLE,             PREFIX,  R2L, "BITWISE_NOT" },  // ~a
  //{ OP_POINTED_VALUE,       2, T_ASTERISK,          PREFIX,  R2L, "POINTED_VALUE" },  // *a
  //{ OP_ADDRESS_OF,          2, T_AMPERSAND,         PREFIX,  R2L, "ADDRESS_OF" },  // &a
    { OP_MULTIPLY,            3, T_ASTERISK,          INFIX,   L2R, "MULTIPLY" },  // *
    { OP_DIVIDE,              3, T_FWD_SLASH,         INFIX,   L2R, "DIVIDE" },  // /
    { OP_MODULO,              3, T_PERCENT,           INFIX,   L2R, "MODULO" },  // %
    { OP_ADD,                 4, T_PLUS,              INFIX,   L2R, "ADD" },  // +
    { OP_SUBTRACT,            4, T_MINUS,             INFIX,   L2R, "SUBTRACT" },  // -
    { OP_LSHIFT,              5, T_DOUBLE_SMALLER,    INFIX,   L2R, "LSHIFT" },  // <<
    { OP_RSHIFT,              5, T_DOUBLE_LARGER,     INFIX,   L2R, "RSHIFT" },  // >>
    { OP_LESS_THAN,           6, T_SMALLER,           INFIX,   L2R, "LESS_THAN" },  // <
    { OP_LESS_EQUAL,          6, T_SMALLER_EQUAL,     INFIX,   L2R, "LESS_EQUAL" },  // <=
    { OP_GREATER_THAN,        6, T_LARGER,            INFIX,   L2R, "GREATER_THAN" },  // >
    { OP_GREATER_EQUAL,       6, T_LARGER_EQUAL,      INFIX,   L2R, "GREATER_EQUAL" },  // >=
    { OP_EQUAL,               7, T_DOUBLE_EQUAL,      INFIX,   L2R, "EQUAL" },  // ==
    { OP_NOT_EQUAL,           7, T_EXCLAMATION_EQUAL, INFIX,   L2R, "NOT_EQUAL" },  // !=
    { OP_BITWISE_AND,         8, T_AMPERSAND,         INFIX,   L2R, "BITWISE_AND" },  // &
    { OP_BITWISE_XOR,         9, T_CARET,             INFIX,   L2R, "BITWISE_XOR" },  // ^
    { OP_BITWISE_OR,         10, T_PIPE,              INFIX,   L2R, "BITWISE_OR" },  // |
    { OP_LOGICAL_AND,        11, T_DOUBLE_AMPERSAND,  INFIX,   L2R, "LOGICAL_AND" },  // &&
    { OP_LOGICAL_OR,         12, T_DOUBLE_PIPE,       INFIX,   L2R, "LOGICAL_OR" },  // ||
    { OP_SHORT_IF,           13, T_QUESTION_MARK,     INFIX,   R2L, "SHORT_IF" },  // a ? b : c
    { OP_ASSIGNMENT,         14, T_EQUAL,             INFIX,   R2L, "ASSIGN" },  // =
    { OP_ADD_ASSIGN,         14, T_PLUS_EQUAL,        INFIX,   R2L, "ADD_ASSIGN" },  // +=
    { OP_SUB_ASSIGN,         14, T_MINUS_EQUAL,       INFIX,   R2L, "SUB_ASSIGN" },  // -=
    { OP_MUL_ASSIGN,         14, T_STAR_EQUAL,        INFIX,   R2L, "MUL_ASSIGN" },  // *=
    { OP_DIV_ASSIGN,         14, T_SLASH_EQUAL,       INFIX,   R2L, "DIV_ASSIGN" },  // /=
    { OP_MOD_ASSIGN,         14, T_PERCENT_EQUAL,     INFIX,   R2L, "MOD_ASSIGN" },  // %=
    { OP_RSH_ASSIGN,         14, T_DBL_LARGER_EQUAL,  INFIX,   R2L, "RSH_ASSIGN" },  // >>=
    { OP_LSH_ASSIGN,         14, T_DBL_SMALLER_EQUAL, INFIX,   R2L, "LSH_ASSIGN" },  // <<=
    { OP_AND_ASSIGN,         14, T_AMPERSAND_EQUAL,   INFIX,   R2L, "AND_ASSIGN" },  // &=
    { OP_OR_ASSIGN,          14, T_PIPE_EQUAL,        INFIX,   R2L, "OR_ASSIGN" },   // |=
    { OP_XOR_ASSIGN,         14, T_CARET_EQUAL,       INFIX,   R2L, "XOR_ASSIGN" },  // ^=
    { OP_SENTINEL,          999, T_UNKNOWN,           POS_UNKNOWN, L2R, "SENTINEL" },
};

struct op_type_info_per_token_type {
    operator_type prefix_op;
    operator_type infix_op;
    operator_type postfix_op;
};
struct op_type_info_per_operator {
    op_type_position position;
    op_type_associativity associativity;
    int priority;
    const char *name;
};
static struct op_type_info_per_token_type op_type_infos_per_token_type[T_MAX_VALUE + 1];
static struct op_type_info_per_operator   op_type_infos_per_operator[OP_MAX_VALUE + 1];

void initialize_operator_type_tables() {
    memset(op_type_infos_per_token_type, 0, sizeof(op_type_infos_per_token_type));
    memset(op_type_infos_per_operator, 0, sizeof(op_type_infos_per_operator));

    for (int i = 0; i < sizeof(operators_flat_list)/sizeof(operators_flat_list[0]); i++) {
        struct op_type_info *info = &operators_flat_list[i];

        if      (info->position == PREFIX)  op_type_infos_per_token_type[info->token_type].prefix_op  = info->op;
        else if (info->position == INFIX)   op_type_infos_per_token_type[info->token_type].infix_op   = info->op;
        else if (info->position == POSTFIX) op_type_infos_per_token_type[info->token_type].postfix_op = info->op;

        op_type_infos_per_operator[info->op].position       = info->position;
        op_type_infos_per_operator[info->op].associativity  = info->associativity;
        op_type_infos_per_operator[info->op].priority       = info->priority;
        op_type_infos_per_operator[info->op].name           = info->name;
    }
}

operator_type operator_type_by_token_and_position(token_type type, enum op_type_position position) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // or '(' in prefix is subexpression, in infix it's function call.
    struct op_type_info_per_token_type *info = &op_type_infos_per_token_type[type];
    if      (position == PREFIX)  return info->prefix_op;
    else if (position == INFIX)   return info->infix_op;
    else if (position == POSTFIX) return info->postfix_op;
    else return OP_UNKNOWN;
}

int operator_type_precedence(operator_type op) {
    return op_type_infos_per_operator[op].priority;
}

op_type_position operator_type_position(operator_type op) {
    return op_type_infos_per_operator[op].position;
}

op_type_associativity operator_type_associativity(operator_type op) {
    return op_type_infos_per_operator[op].associativity;
}

bool operator_type_is_unary(operator_type op) {
    op_type_position pos = operator_type_position(op);
    return pos == PREFIX || pos == POSTFIX;
}

const char *operator_type_to_string(operator_type op) {
    return op_type_infos_per_operator[op].name;
}

bool operator_types_are_equal(operator_type a, operator_type b) {
    return a == b;
}

contained_item *containing_operator_types = &(contained_item){
    .type_name = "operator_type",
    .are_equal = (are_equal_func)operator_types_are_equal,
    .to_string = (to_string_func)operator_type_to_string
};
