#include <string.h>
#include "operator.h"
#include "token.h"

struct op_info {
    token_type token_type;
    op_position position;
    operator op;
    int operands_count;
    int priority;
};

// first entries more precedent than later entries
static struct op_info operators_flat_list[] = {
    { T_IDENTIFIER,       OPERAND,   OP_SYMBOL_VALUE,     0,  1 },
    { T_NUMBER_LITERAL,   OPERAND,   OP_NUMBER_VALUE,     0,  1 },
    { T_PLUS,             INFIX,     OP_ADDITION,         2,  4 },
    { T_PLUS,             PREFIX,    OP_POSITIVE_NUM,     1,  1 },
    { T_MINUS,            INFIX,     OP_SUBTRACTION,      2,  4 },
    { T_MINUS,            PREFIX,    OP_NEGATIVE_NUM,     1,  1 },
    { T_ASTERISK,         INFIX,     OP_MULTIPLICATION,   2,  2 },
    { T_FWD_SLASH,        INFIX,     OP_DIVISION,         2,  2 },
    { T_AMPERSAND,        INFIX,     OP_BINARY_AND,       2,  1 },
    { T_PIPE,             INFIX,     OP_BINARY_OR,        2,  1 },
    { T_TIDLE,            PREFIX,    OP_BINARY_NOT,       1,  1 },
    { T_DOUBLE_AMPERSAND, INFIX,     OP_LOGICAL_AND,      2,  1 },
    { T_DOUBLE_PIPE,      INFIX,     OP_LOGICAL_OR,       2,  1 },
    { T_EXCLAMATION,      PREFIX,    OP_LOGICAL_NOT,      1,  1 },
};

struct op_info_per_operator {
    op_position position;
    int operands_count;
    int priority;
};

struct op_info_per_token_type {
    operator operand_op;
    operator prefix_op;
    operator infix_op;
    operator postfix_op;
};
static struct op_info_per_token_type op_infos_per_token_type[T_MAX_VALUE + 1];
static struct op_info_per_operator   op_infos_per_operator[OP_MAX_VALUE + 1];

void initialize_operator_tables() {
    memset(op_infos_per_token_type, 0, sizeof(op_infos_per_token_type));
    memset(op_infos_per_operator, 0, sizeof(op_infos_per_operator));

    for (int i = 0; i < sizeof(operators_flat_list)/sizeof(operators_flat_list[0]); i++) {
        struct op_info *info = &operators_flat_list[i];

        if      (info->position == OPERAND) op_infos_per_token_type[info->token_type].operand_op = info->op;
        else if (info->position == PREFIX)  op_infos_per_token_type[info->token_type].prefix_op  = info->op;
        else if (info->position == INFIX)   op_infos_per_token_type[info->token_type].infix_op   = info->op;
        else if (info->position == POSTFIX) op_infos_per_token_type[info->token_type].postfix_op = info->op;

        op_infos_per_operator[info->op].position       = info->position;
        op_infos_per_operator[info->op].operands_count = info->operands_count;
        op_infos_per_operator[info->op].priority       = info->priority;
    }
}

operator get_operator_by_token_type_and_position(token_type type, enum op_position position) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    struct op_info_per_token_type *info = &op_infos_per_token_type[type];
    if      (position == OPERAND) return info->operand_op;
    else if (position == PREFIX)  return info->prefix_op;
    else if (position == INFIX)   return info->infix_op;
    else if (position == POSTFIX) return info->postfix_op;
    else return OP_UNKNOWN;
}

int operator_precedence(operator op) {
    return (op == OP_SENTINEL) ? 
        LOWEST_OPERATOR_PRECEDENCE :
        op_infos_per_operator[op].priority;
}

op_position operator_position(operator op) {
    return op_infos_per_operator[op].position;
}

int operator_operands_count(operator op) {
    return op_infos_per_operator[op].operands_count;
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
