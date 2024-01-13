#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/strbld.h"
#include "../../utils/failable.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/stack.h"
#include "parser.h"
#include "../lexer/token.h"
#include "../lexer/tokenization.h"
#include "operator.h"
#include "expression.h"

/*
    The shunting yard algorithm, for parsing operators according to precedence,
    https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard
    
    Insightful answer here: https://stackoverflow.com/questions/16380234
    General format: PREFIX_OP* OPERAND POSTFIX_OP* (INFIX_OP PREFIX_OP* OPERAND POSTFIX_OP*)*

    +---------+                     +---------+  
    |  WANT   | ------OPERAND-----> |  HAVE   | 
    | OPERAND | <---INFIX_OPERTR--- | OPERAND |
    |         |---+                 |         |---+
    +---------+   |                 +---------+   |
         ^      PREFIX                   ^     POSTFIX
         |      OPERTR                   |      OPERTR
         +--------+                      +--------+
*/

typedef enum run_state { WANT_OPERAND, HAVE_OPERAND, FINISHED } run_state;
typedef enum completion_mode { CM_NORMAL, CM_SUB_EXPRESSION, CM_FUNC_ARGS, CM_COLON } completion_mode;


static stack *operators_stack;
static stack *expressions_stack;
static iterator *tokens_iterator;
static token *last_token;
static token *end_token;
static failable_expression parse_expression(completion_mode completion, bool verbose);


static token* get_token_and_advance() {
    // get token, advance to next position, so we can peek.
    if (!tokens_iterator->valid(tokens_iterator))
        return end_token;
    
    last_token = tokens_iterator->curr(tokens_iterator);
    tokens_iterator->next(tokens_iterator);
    return last_token;
}

static token* peek_token() {
    // get token, but don't advance to next position
    if (!tokens_iterator->valid(tokens_iterator))
        return end_token;
    
    return tokens_iterator->curr(tokens_iterator);
}

static inline operator get_token_prefix_operator(token_type tt) {
    return get_operator_by_token_type_and_position(tt, PREFIX);
}

static inline operator get_token_infix_operator(token_type tt) {
    return get_operator_by_token_type_and_position(tt, INFIX);
}

static inline operator get_token_postfix_operator(token_type tt) {
    return get_operator_by_token_type_and_position(tt, POSTFIX);
}

static inline bool is_token_operand(token_type tt) {
    return tt == T_IDENTIFIER ||
           tt == T_NUMBER_LITERAL ||
           tt == T_STRING_LITERAL ||
           tt == T_BOOLEAN_LITERAL;
}

static inline expression *get_operand_expression(token_type tt, const char *data) {
    switch (tt) {
        case T_IDENTIFIER: return new_identifier_expression(data);
        case T_NUMBER_LITERAL: return new_numeric_literal_expression(data);
        case T_STRING_LITERAL: return new_string_literal_expression(data);
        case T_BOOLEAN_LITERAL: return new_boolean_literal_expression(data);
    }
    return NULL;
}

static inline bool is_token_prefix_operator(token_type tt) {
    return get_token_prefix_operator(tt) != T_UNKNOWN;
}

static inline bool is_token_infix_operator(token_type tt) {
    return get_token_infix_operator(tt) != T_UNKNOWN;
}

static inline bool is_token_postfix_operator(token_type tt) {
    return get_token_postfix_operator(tt) != T_UNKNOWN;
}

static inline void push_operator_for_later(operator op) {
    stack_push(operators_stack, (void *)op);
}

static inline operator peek_top_operator() {
    return (operator)stack_peek(operators_stack);
}

static inline operator pop_top_operator() {
    return (operator)stack_pop(operators_stack);
}

static void print_operators_stack(FILE *stream, char *prefix) {
    fprintf(stream, "%sOperators stack (%d)\n", prefix, stack_length(operators_stack));
    int i = stack_length(operators_stack) - 1;
    for_stack(operators_stack, it, void, ptr) {
        fprintf(stream, "%s    %d: %s\n", prefix, i--, operator_str((operator)ptr));
    }
}

static inline void push_expression(expression *e) {
    return stack_push(expressions_stack, e);
}

static inline expression *pop_top_expression() {
    return (expression *)stack_pop(expressions_stack);
}

static inline expression *peek_top_expression() {
    return (expression *)stack_peek(expressions_stack);
}

static void print_expressions_stack(FILE *stream, char *prefix) {
    fprintf(stream, "%sExpressions stack (%d)\n", prefix, stack_length(expressions_stack));
    if (stack_empty(expressions_stack)) {
        fprintf(stream, "%s    (empty)\n", prefix);
    } else {
        strbld *separator_sb = new_strbld();
        strbld_catf(separator_sb, "\n%s    ", prefix);
        const char *str = stack_to_string(expressions_stack, strbld_charptr(separator_sb));
        fprintf(stream, "%s    %s\n", prefix, str);
    }
}

// --------------------------------------------

static void make_one_expression_from_top_operator() {
    operator op = pop_top_operator();
    expression *new_expr;

    op_position pos = operator_position(op);
    if (pos == PREFIX || pos == POSTFIX) {
        expression *operand1 = pop_top_expression();
        new_expr = new_unary_op_expression(op, operand1);
    } else if (pos == INFIX) {
        expression *operand2 = pop_top_expression();
        expression *operand1 = pop_top_expression();
        new_expr = new_binary_op_expression(op, operand1, operand2);
    }
    push_expression(new_expr);
}

static void create_expressions_for_higher_operators_than(operator op) {
    // the operators stack always has the highest precedence ops at the top.
    // if we want to add a smaller precedence, we pop them into expressions
    // this assumes the use of the SENTINEL, the lowest priority operator
    int precedence = operator_precedence(op);
    operator top_op = peek_top_operator();
    while (operator_precedence(top_op) < precedence) {
        make_one_expression_from_top_operator();
        top_op = peek_top_operator();
    }
}

// --------------------------------------------

static failable_bool detect_completion(token_type curr_token, completion_mode completion) {
    if (completion == CM_NORMAL) {
        return ok_bool(curr_token == T_END || curr_token == T_SEMICOLON);
    }

    // all other modes should not end prematurely.
    if (curr_token == T_END || curr_token == T_SEMICOLON)
        return failed("Unexpected end of expression (%s), when completion mode is %d", 
                token_type_str(curr_token), completion);
    
    if (completion == CM_SUB_EXPRESSION) {
        return ok_bool(curr_token == T_RPAREN);
    } else if (completion == CM_FUNC_ARGS) {
        return ok_bool(curr_token == T_RPAREN || curr_token == T_COMMA);
    } else if (completion == CM_COLON) {
        return ok_bool(curr_token == T_COLON);
    }

    return failed("Unknown completion mode %d", completion);
}

static failable parse_expression_on_want_operand(run_state *state, bool verbose) {
    // read a token. If there are no more tokens, announce an error.
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);

    // handle prefix ops 
    if (is_token_prefix_operator(tt)) {
        operator op = get_token_prefix_operator(tt);
        push_operator_for_later(op);
        return ok();
    }

    // handle sub-expressions, func cals are handled after having operand.
    if (tt == T_LPAREN) {
        failable_expression sub_expression = parse_expression(CM_SUB_EXPRESSION, verbose);
        if (sub_expression.failed)
            return failed("Subexpression failed: %s", sub_expression.err_msg);
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // handle operands
    if (is_token_operand(tt)) {
        expression *e = get_operand_expression(tt, token_get_data(t));
        push_expression(e);
        *state = HAVE_OPERAND;
        return ok();
    }

    // nothing else should be expected here
    return failed("Unexpected token type %s, was expecting prefix, operand, or lparen", token_type_str(tt));
}

static failable_list parse_function_arguments_expressions(bool verbose) {
    list *args = new_list();

    // if empty args, there will be nothing to parse
    if (token_get_type(peek_token()) == T_RPAREN) {
        get_token_and_advance();
        return ok_list(args);
    }

    while (token_get_type(last_token) != T_RPAREN) {
        failable_expression parse_arg = parse_expression(CM_FUNC_ARGS, verbose);
        if (parse_arg.failed)
            return failed("%s", parse_arg.err_msg);
        list_add(args, parse_arg.result);
    }
    
    return ok_list(args);
}

failable_expression parse_shorthand_if_pair(bool verbose) {
    failable_expression parsing = parse_expression(CM_COLON, verbose);
    if (parsing.failed) return failed("%s", parsing.err_msg);
    expression *e1 = parsing.result;

    parsing = parse_expression(CM_NORMAL, verbose);
    if (parsing.failed) return failed("%s", parsing.err_msg);
    expression *e2 = parsing.result;

    return ok_expression(new_pair_expression(e1, e2));
}


static failable parse_expression_on_have_operand(run_state *state, completion_mode completion, bool verbose) {
    // read a token   
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);

    // if postfix, push for later, and remain in state
    if (is_token_postfix_operator(tt)) {
        operator op = get_token_postfix_operator(tt);
        create_expressions_for_higher_operators_than(op);
        push_operator_for_later(op);
        return ok();
    }

    // handle function calls, operand is function name or address
    if (tt == T_LPAREN) {
        failable_list arg_expressions = parse_function_arguments_expressions(verbose);
        if (arg_expressions.failed)
            return failed("%s", arg_expressions.err_msg);
        push_expression(new_func_args_expression(arg_expressions.result));
        create_expressions_for_higher_operators_than(OP_FUNC_CALL);
        push_operator_for_later(OP_FUNC_CALL);
        *state = HAVE_OPERAND;
        return ok();
    }

    if (tt == T_QUESTION) {
        failable_expression if_parts = parse_shorthand_if_pair(verbose);
        if (if_parts.failed)
            return failed("%s", if_parts.err_msg);
        create_expressions_for_higher_operators_than(OP_SHORT_IF);
        push_operator_for_later(OP_SHORT_IF);
        push_expression(if_parts.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // if infix, push it for resolving later, and go back to want-operand
    if (is_token_infix_operator(tt)) {
        operator op = get_token_infix_operator(tt);
        create_expressions_for_higher_operators_than(op);
        push_operator_for_later(op);
        *state = WANT_OPERAND;
        return ok();
    }

    // detect if we finished (we may be a sub-expression)
    failable_bool completion_detection = detect_completion(tt, completion);
    if (completion_detection.failed)
        return failed("%s", completion_detection.err_msg);
    if ((bool)completion_detection.result) {
        create_expressions_for_higher_operators_than(OP_SENTINEL);
        *state = FINISHED;
        return ok();
    }
    
    return ok();
}

static void print_debug_information(char *title, run_state state) {
        fprintf(stderr, "%s\n", title);

        char *state_name;
        switch (state) {
            case WANT_OPERAND: state_name = "WANT_OPERAND"; break;
            case HAVE_OPERAND: state_name = "HAVE_OPERAND"; break;
            case FINISHED: state_name = "FINISHED"; break;
            default: state_name = "(unknown)";
        }
        fprintf(stderr, "    Curr state %s", state_name);

        fprintf(stderr, ", last token "); 
        if (last_token == NULL)
            fprintf(stderr, "(null)");
        else
            token_print(last_token, stderr, "");

        fprintf(stderr, ", next token ");
        token_print(peek_token(), stderr, "");
        fprintf(stderr, "\n");

        print_expressions_stack(stderr, "    ");
        print_operators_stack(stderr, "    ");
}

static failable_expression parse_expression(completion_mode completion, bool verbose) {
    // re-entrable, for subexpressions.
    // parse till we encounter the completion condition

    failable state_handling;
    run_state state = WANT_OPERAND;
    push_operator_for_later(OP_SENTINEL);

    if (verbose)
        print_debug_information("will enter parse_expression() loop", state);
    while (state != FINISHED) {

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state, verbose);
                if (state_handling.failed)
                    return failed("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, completion, verbose);
                if (state_handling.failed)
                    return failed("Failed on have operand: %s", state_handling.err_msg);
                break;
        }

        if (verbose)
            print_debug_information("after one instanceparse_expression() loop", state);
    }

    if (peek_top_operator() != OP_SENTINEL)
        return failed("Was expecting SENTINEL at the top of the queue");
    pop_top_operator();

    return ok_expression(pop_top_expression());
}

failable_list parse_tokens_into_expressions(list *tokens, bool verbose) {
    operators_stack = new_stack();
    expressions_stack = new_stack();
    tokens_iterator = list_iterator(tokens);
    end_token = new_token(T_END);
    last_token = new_token(T_UNKNOWN);

    // there may be more than one expressions, parse them as long as we have tokens
    list *expressions = new_list();
    tokens_iterator->reset(tokens_iterator);
    while (tokens_iterator->valid(tokens_iterator) && token_get_type(tokens_iterator->curr(tokens_iterator)) != T_END) {
        failable_expression parsing = parse_expression(CM_NORMAL, verbose);
        if (parsing.failed)
            return failed("%s", parsing.err_msg);

        list_add(expressions, parsing.result);
    }

    return ok_list(expressions);
}
