#include <string.h>
#include "operator.h"

struct op_info {
    operator op;
    int priority;
    token_type token_type;
    op_position position;
    const char *name;
};

// first entries more precedent than later entries
static struct op_info operators_flat_list[] = {
    { OP_POST_INC,            10, T_DOUBLE_PLUS,       POSTFIX, "POST_INC" },  // a++
    { OP_POST_DEC,            10, T_DOUBLE_MINUS,      POSTFIX, "POST_DEC" },  // a--
    { OP_ARRAY_SUBSCRIPT,     10, T_LSQBRACKET,        INFIX,   "ARRAY_SUBSCRIPT" },  // a[b]
  //{ OP_STRUCT_MEMBER_PTR,   10, T_ARROW,             INFIX,   "STRUCT_MEMBER_PTR" },  // a->b
    { OP_MEMBER,              10, T_DOT,               INFIX,   "MEMBER" },  // a.b
    { OP_FUNC_CALL,           10, T_LPAREN,            INFIX,   "CALL" },  // a()
    { OP_PRE_INC,             20, T_DOUBLE_PLUS,       PREFIX,  "PRE_INC" },  // ++a
    { OP_PRE_DEC,             20, T_DOUBLE_MINUS,      PREFIX,  "PRE_DEC" },  // --a
    { OP_POSITIVE_NUM,        20, T_PLUS,              PREFIX,  "POSITIVE_NUM" },  // +123
    { OP_NEGATIVE_NUM,        20, T_MINUS,             PREFIX,  "NEGATIVE_NUM" },  // -123
    { OP_LOGICAL_NOT,         20, T_EXCLAMATION,       PREFIX,  "LOGICAL_NOT" },  // !a
    { OP_BITWISE_NOT,         20, T_TIDLE,             PREFIX,  "BITWISE_NOT" },  // ~a
  //{ OP_POINTED_VALUE,       20, T_ASTERISK,          PREFIX,  "POINTED_VALUE" },  // *a
  //{ OP_ADDRESS_OF,          20, T_AMPERSAND,         PREFIX,  "ADDRESS_OF" },  // &a
    { OP_MULTIPLY,            30, T_ASTERISK,          INFIX,   "MULTIPLY" },  // *
    { OP_DIVIDE,              30, T_FWD_SLASH,         INFIX,   "DIVIDE" },  // /
    { OP_MODULO,              30, T_PERCENT,           INFIX,   "MODULO" },  // %
    { OP_ADD,                 40, T_PLUS,              INFIX,   "ADD" },  // +
    { OP_SUBTRACT,            40, T_MINUS,             INFIX,   "SUBTRACT" },  // -
    { OP_LSHIFT,              50, T_DOUBLE_SMALLER,    INFIX,   "LSHIFT" },  // <<
    { OP_RSHIFT,              50, T_DOUBLE_LARGER,     INFIX,   "RSHIFT" },  // >>
    { OP_LESS_THAN,           60, T_SMALLER,           INFIX,   "LESS_THAN" },  // <
    { OP_LESS_EQUAL,          60, T_SMALLER_EQUAL,     INFIX,   "LESS_EQUAL" },  // <=
    { OP_GREATER_THAN,        60, T_LARGER,            INFIX,   "GREATER_THAN" },  // >
    { OP_GREATER_EQUAL,       60, T_LARGER_EQUAL,      INFIX,   "GREATER_EQUAL" },  // >=
    { OP_EQUAL,               70, T_DOUBLE_EQUAL,      INFIX,   "EQUAL" },  // ==
    { OP_NOT_EQUAL,           70, T_EXCLAMATION_EQUAL, INFIX,   "NOT_EQUAL" },  // !=
    { OP_BITWISE_AND,         80, T_AMPERSAND,         INFIX,   "BITWISE_AND" },  // &
    { OP_BITWISE_XOR,         90, T_CARET,             INFIX,   "BITWISE_XOR" },  // ^
    { OP_BITWISE_OR,         100, T_PIPE,              INFIX,   "BITWISE_OR" },  // |
    { OP_LOGICAL_AND,        110, T_DOUBLE_AMPERSAND,  INFIX,   "LOGICAL_AND" },  // &&
    { OP_LOGICAL_OR,         120, T_DOUBLE_PIPE,       INFIX,   "LOGICAL_OR" },  // ||
    { OP_SHORT_IF,           130, T_QUESTION,          INFIX,   "SHORT_IF" },  // a ? b : c
    { OP_ASSIGNMENT,         140, T_EQUAL,             INFIX,   "ASSIGNMENT" },  // =
    { OP_ADD_ASSIGN,         140, T_PLUS_EQUAL,        INFIX,   "ADD_ASSIGN" },  // +=
    { OP_SUB_ASSIGN,         140, T_MINUS_EQUAL,       INFIX,   "SUB_ASSIGN" },  // -=
    { OP_MUL_ASSIGN,         140, T_STAR_EQUAL,        INFIX,   "MUL_ASSIGN" },  // *=
    { OP_DIV_ASSIGN,         140, T_SLASH_EQUAL,       INFIX,   "DIV_ASSIGN" },  // /=
    { OP_MOD_ASSIGN,         140, T_PERCENT_EQUAL,     INFIX,   "MOD_ASSIGN" },  // %=
    { OP_RSH_ASSIGN,         140, T_DBL_LARGER_EQUAL,  INFIX,   "RSH_ASSIGN" },  // >>=
    { OP_LSH_ASSIGN,         140, T_DBL_SMALLER_EQUAL, INFIX,   "LSH_ASSIGN" },  // <<=
    { OP_AND_ASSIGN,         140, T_AMPERSAND_EQUAL,   INFIX,   "AND_ASSIGN" },  // &=
    { OP_OR_ASSIGN,          140, T_PIPE_EQUAL,        INFIX,   "OR_ASSIGN" },   // |=
    { OP_XOR_ASSIGN,         140, T_CARET_EQUAL,       INFIX,   "XOR_ASSIGN" },  // ^=
    { OP_SENTINEL,          9990, T_UNKNOWN,           POS_UNKNOWN, "SENTINEL" },
};

struct op_info_per_token_type {
    operator prefix_op;
    operator infix_op;
    operator postfix_op;
};
struct op_info_per_operator {
    op_position position;
    int priority;
    const char *name;
};
static struct op_info_per_token_type op_infos_per_token_type[T_MAX_VALUE + 1];
static struct op_info_per_operator   op_infos_per_operator[OP_MAX_VALUE + 1];

void initialize_operator_tables() {
    memset(op_infos_per_token_type, 0, sizeof(op_infos_per_token_type));
    memset(op_infos_per_operator, 0, sizeof(op_infos_per_operator));

    for (int i = 0; i < sizeof(operators_flat_list)/sizeof(operators_flat_list[0]); i++) {
        struct op_info *info = &operators_flat_list[i];

        if      (info->position == PREFIX)  op_infos_per_token_type[info->token_type].prefix_op  = info->op;
        else if (info->position == INFIX)   op_infos_per_token_type[info->token_type].infix_op   = info->op;
        else if (info->position == POSTFIX) op_infos_per_token_type[info->token_type].postfix_op = info->op;

        op_infos_per_operator[info->op].position       = info->position;
        op_infos_per_operator[info->op].priority       = info->priority;
        op_infos_per_operator[info->op].name           = info->name;
    }
}

operator get_operator_by_token_type_and_position(token_type type, enum op_position position) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // or '(' in prefix is subexpression, in infix it's function call.
    struct op_info_per_token_type *info = &op_infos_per_token_type[type];
    if      (position == PREFIX)  return info->prefix_op;
    else if (position == INFIX)   return info->infix_op;
    else if (position == POSTFIX) return info->postfix_op;
    else return OP_UNKNOWN;
}

int operator_precedence(operator op) {
    return op_infos_per_operator[op].priority;
}

op_position operator_position(operator op) {
    return op_infos_per_operator[op].position;
}

const char *operator_str(operator op) {
    return op_infos_per_operator[op].name;
}

bool operators_are_equal(operator a, operator b) {
    return a == b;
}

contained_item *containing_operators = &(contained_item){
    .type_name = "operator",
    .are_equal = (are_equal_func)operators_are_equal,
    .to_string = (to_string_func)operator_str
};
