#include <stdbool.h>
#include "../utils/failable.h"
#include "../utils/list.h"
#include "../utils/stack.h"
#include "parser.h"
#include "token.h"
#include "operator.h"
#include "expression.h"

/*
    The shunting yard algorithm, for parsing operators according to precedence,
    https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard
    
    Very good answer here: https://stackoverflow.com/questions/16380234
    General format: PREFIX_OP* OPERAND POSTFIX_OP* (INFIX_OP PREFIX_OP* OPERAND POSTFIX_OP*)*

    +-----+            +-----+              
    |State|-----OP---->|State|              State 1: want operand
    |  1  |<----INF----|  2  |              State 2: have operand
    |     |---+        |     |---+
    +-----+   |        +-----+   |
        ^     PRE          ^     POST
        |      |           |      |
        +------+           +------+
*/

enum states { WANT_OPERAND, HAVE_OPERAND, DONE };
static stack *operators_stack;
static stack *expressions_stack;
static iterator *tokens_iterator;
static token *end_token;


static token* get_token_and_advance() {
    // get token, advance to next position, so we can peek.
    if (!iterator_valid(tokens_iterator)) {
        return end_token;
    }
    token *t = iterator_current(tokens_iterator);
    tokens_iterator = iterator_next(tokens_iterator);
    return t;
}

static token* peek_token() {
    // get token, but don't advance to next position
    if (!iterator_valid(tokens_iterator)) {
        return end_token;
    }
    return iterator_current(tokens_iterator);
}

static inline operator get_token_operand_operator(token_type tt) {
    return get_operator_by_token_type_and_position(tt, OPERAND);
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

static inline bool is_operand_token(token_type tt) {
    return get_token_operand_operator(tt) != T_UNKNOWN;
}

static inline bool is_prefix_operator_token(token_type tt) {
    return get_token_prefix_operator(tt) != T_UNKNOWN;
}

static inline bool is_infix_operator_token(token_type tt) {
    return get_token_infix_operator(tt) != T_UNKNOWN;
}

static inline bool is_postfix_operator_token(token_type tt) {
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

static inline void push_expression(expression *e) {
    return stack_push(expressions_stack, e);
}

static inline expression *pop_top_expression() {
    return (expression *)stack_pop(expressions_stack);
}

static inline expression *peek_top_expression() {
    return (expression *)stack_peek(expressions_stack);
}

static void make_one_expression_from_top_operator() {
    operator op = pop_top_operator();
    expression *new_expr;

    op_position pos = operator_position(op);
    if (pos == PREFIX || pos == POSTFIX) {
        expression *operand1 = pop_top_expression();
        new_expr = new_unary_expression(op, operand1);
    } else if (pos == INFIX) {
        expression *operand2 = pop_top_expression();
        expression *operand1 = pop_top_expression();
        new_expr = new_binary_expression(op, operand1, operand2);
    }
    push_expression(new_expr);
}
static void resolve_pending_higher_precedence_operators(int precedence) {
    // the operators stack always has the highest precedence ops at the top.
    // if we want to add a smaller precedence, we pop them into expressions
    // this assumes the use of the SENTINEL, the lowest priority operator
    operator top_op = peek_top_operator();
    while (operator_precedence(top_op) > precedence) {
        make_one_expression_from_top_operator();
        top_op = peek_top_operator();
    }
}




static failable parse_expression_on_want_operand(enum states *state) {
    // read a token. If there are no more tokens, announce an error.
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);
    if (tt == T_END) {
        return failed("Premature end of tokens");
    }


    // if the token is an prefix operator or an '(':
    //     mark it as prefix and push it onto the operator stack
    //     goto want_operand
    // if the token is an operand (identifier or variable):
    //     add it to the output queue
    //     goto have_operand
    // if the token is anything else, announce an error and stop. (**)    
    return succeeded();
}

static failable parse_expression_on_have_operand(enum states *state) {
    // read a token   
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);

    // if there are no more tokens (or end-of-expression symbol):
    //     pop all operators off the stack, adding each one to the output queue.
    //     if a `(` is found on the stack, announce an error and stop.
    // if the token is a postfix operator:
    //     mark it as postfix and add it to the output queue
    //     goto have_operand.
    // if the token is a ')':
    //     while the top of the stack is not '(':
    //     pop an operator off the stack and add it to the output queue
    //     if the stack becomes empty, announce an error and stop.
    //     if the '(' is marked infix, add a "call" operator to the output queue (*)
    //     pop the '(' off the top of the stack
    //     goto have_operand
    // if the token is a ',':
    //     while the top of the stack is not '(':
    //     pop an operator off the stack and add it to the output queue
    //     if the stack becomes empty, announce an error
    //     goto want_operand
    // if the token is an infix operator:
    //     (see the wikipeda entry for precedence handling)
    //     (normally, all prefix operators are considered to have higher precedence
    //     than infix operators.)
    //     got to want_operand
    // otherwise, token is an operand. Announce an error
    return succeeded();
}

static failable_expression parse_expression() {
    // re-entrable, for subexpressions.
    // parse till we meet:
    // - T_END, or, end of expression ';'
    // - possible ternary chars: '?' or ':'
    // - possible RPAREN, for parsing subexpression

    failable state_handling;
    enum states state = WANT_OPERAND;
    
    while (state != DONE) {
        if (token_get_type(peek_token()) == T_END) {
            return failed_expression("Premature end of tokens");
        }

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state);
                if (state_handling.failed)
                    return failed_expression("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state);
                if (state_handling.failed)
                    return failed_expression("Failed on have operand: %s", state_handling.err_msg);
                break;
        }
    }

    return ok_expression(pop_top_expression());
}

failable_list parse_tokens_into_expressions(list *tokens) {
    operators_stack = new_stack();
    expressions_stack = new_stack();
    tokens_iterator = list_iterator(tokens);
    end_token = new_token(T_END);

    // there may be more than one expressions, parse them as long as we have tokens
    list *expressions = new_list();
    while (iterator_valid(tokens_iterator)) {
        failable_expression parsing = parse_expression();
        if (parsing.failed)
            return failed_list("Error parsing: %s", parsing.err_msg);

        list_add(expressions, parsing.result);
    }

    return ok_list(expressions);
}
