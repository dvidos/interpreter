#include <stdbool.h>
#include "../utils/failable.h"
#include "../utils/list.h"
#include "../utils/stack.h"
#include "parser.h"
#include "token.h"
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
static stack *operands_stack;
static stack *expressions_stack;
static iterator *tokens_iterator;
static token *end_token;

static token* get_next_token() {
    if (!iterator_valid(tokens_iterator)) {
        return end_token;
    }
    token *t = iterator_current(tokens_iterator);
    iterator_next(tokens_iterator);
    return t;
}

static bool is_operand(token *t) {
    token_type tt = token_get_type(t);
    return (tt == T_IDENTIFIER ||
            tt == T_NUMBER_LITERAL ||
            tt == T_STRING_LITERAL ||
            tt == T_BOOLEAN_LITERAL);
}

static operator get_possible_prefix_operator(token *token) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // return operator or OP_UNKNOWN
}

static operator get_possible_infix_operator(token *token) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // return operator or OP_UNKNOWN
}

static operator get_possible_postfix_operator(token *token) {
    // for example, '-' can be prefix/infix, '++' can be prefix/postfix
    // return operator or OP_UNKNOWN
}

static failable handle_want_operand_state(enum states *state) {
    // read a token. If there are no more tokens, announce an error.
    token *t = get_next_token();
    if (token_get_type(t) == T_END)
        return failed("tokens finished prematurely");
    
    if (token_get_type(t) == T_LPAREN) {

    }
    
    if (is_operand(t)) {

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

static failable handle_have_operand_state(enum states *state) {
    // read a token   
    token *t = get_next_token();

    // if there are no more tokens:
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

static failable_expression parse_full_expression() {

    failable handling;
    operands_stack = new_stack();
    expressions_stack = new_stack();
    enum states state = WANT_OPERAND;
    
    while (state != DONE) {
        switch (state) {
            case WANT_OPERAND:
                handling = handle_want_operand_state(&state);
                if (handling.failed)
                    return failed_expression("Want operand failed: %s", handling.err_msg);
                break;
            case HAVE_OPERAND:
                handling = handle_have_operand_state(&state);
                if (handling.failed)
                    return failed_expression("Have operand failed: %s", handling.err_msg);
                break;
        }
    }

    expression *e = stack_pop(expressions_stack);
    return ok_expression(e);
}

failable_list parse_tokens_into_expressions(list *tokens) {
    tokens_iterator = list_iterator(tokens);
    end_token = new_token(T_END);

    // there may be more than one expressions, parse them as long as we have tokens
    list *expressions = new_list();
    while iterator_valid(tokens_iterator) {
        failable_expression parsing = parse_full_expression();
        if (parsing.failed)
            return failed_list("Error parsing: %s", parsing.err_msg);

        list_add(expressions, parsing.result);
    }

    return ok_list(expressions);
}
