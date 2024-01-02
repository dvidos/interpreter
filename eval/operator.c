#include <string.h>
#include "operator.h"
#include "token.h"

struct op_info {
    token_type token_type;
    op_position position;
    operator op;
    int operands_count;
    int priority;
    const char *name;
};

// first entries more precedent than later entries
static struct op_info operators_flat_list[] = {
    { T_IDENTIFIER,       OPERAND,   OP_SYMBOL_VALUE,     0,  1, "IDENTIFIER" },
    { T_NUMBER_LITERAL,   OPERAND,   OP_NUMBER_VALUE,     0,  1, "NUMBER_LITERAL" },
    { T_PLUS,             INFIX,     OP_ADDITION,         2,  4, "PLUS" },
    { T_PLUS,             PREFIX,    OP_POSITIVE_NUM,     1,  1, "PLUS" },
    { T_MINUS,            INFIX,     OP_SUBTRACTION,      2,  4, "MINUS" },
    { T_MINUS,            PREFIX,    OP_NEGATIVE_NUM,     1,  1, "MINUS" },
    { T_ASTERISK,         INFIX,     OP_MULTIPLICATION,   2,  2, "ASTERISK" },
    { T_FWD_SLASH,        INFIX,     OP_DIVISION,         2,  2, "FWD_SLASH" },
    { T_AMPERSAND,        INFIX,     OP_BINARY_AND,       2,  1, "AMPERSAND" },
    { T_PIPE,             INFIX,     OP_BINARY_OR,        2,  1, "PIPE" },
    { T_TIDLE,            PREFIX,    OP_BINARY_NOT,       1,  1, "TIDLE" },
    { T_DOUBLE_AMPERSAND, INFIX,     OP_LOGICAL_AND,      2,  1, "DOUBLE_AMPERSAND" },
    { T_DOUBLE_PIPE,      INFIX,     OP_LOGICAL_OR,       2,  1, "DOUBLE_PIPE" },
    { T_EXCLAMATION,      PREFIX,    OP_LOGICAL_NOT,      1,  1, "EXCLAMATION" },
};


struct op_info_per_token_type {
    operator operand_op;
    operator prefix_op;
    operator infix_op;
    operator postfix_op;
};
struct op_info_per_operator {
    op_position position;
    int operands_count;
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

        if      (info->position == OPERAND) op_infos_per_token_type[info->token_type].operand_op = info->op;
        else if (info->position == PREFIX)  op_infos_per_token_type[info->token_type].prefix_op  = info->op;
        else if (info->position == INFIX)   op_infos_per_token_type[info->token_type].infix_op   = info->op;
        else if (info->position == POSTFIX) op_infos_per_token_type[info->token_type].postfix_op = info->op;

        op_infos_per_operator[info->op].position       = info->position;
        op_infos_per_operator[info->op].operands_count = info->operands_count;
        op_infos_per_operator[info->op].priority       = info->priority;
        op_infos_per_operator[info->op].name           = info->name;
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
    return op_infos_per_operator[op].name;
}
