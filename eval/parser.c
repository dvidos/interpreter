#include "../utils/failable.h"
#include "parser.h"
#include "token.h"
#include "expression.h"

/*
    The shunting yard algorithm, for parsing operators according to precedence,
    https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard
    
    Very good answer here: https://stackoverflow.com/questions/16380234
    General format: PREFIX_OP* OPERAND POSTFIX_OP* (INFIX_OP PREFIXOP* OPERAND POSTFIX_OP*)*

    +-----+            +-----+              
    |State|-----OP---->|State|              State 1: want operand
    |  1  |<----INF----|  2  |              State 2: have operand
    |     |---+        |     |---+
    +-----+   |        +-----+   |
        ^     PRE          ^     POST
        |      |           |      |
        +------+           +------+
*/

enum states { WANT_OPERAND, HAVE_OPERAND, COMPLETED };

static void handle_want_operand_state(enum states *state) {
    // read a token. If there are no more tokens, announce an error.
    // if the token is an prefix operator or an '(':
    //     mark it as prefix and push it onto the operator stack
    //     goto want_operand
    // if the token is an operand (identifier or variable):
    //     add it to the output queue
    //     goto have_operand
    // if the token is anything else, announce an error and stop. (**)    
}

static void handle_have_operand_state(enum states *state) {
    // read a token
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
}


static bool parse_full_expression() {
    // e.g. "a+b*c+d"
    enum states state = WANT_OPERAND;
    while (state != COMPLETED) {
        switch (state) {
            case WANT_OPERAND:
                handle_want_operand_state(&state);
                break;
            case HAVE_OPERAND:
                handle_have_operand_state(&state);
                break;
        }
    }
    return false;
}




failable_list parse_tokens_into_expressions(list *tokens) {
    // parse the tokens into an AST. Need operation precedence here & double stack.
    // expression can be:
    //    <operand> [ <operator> <operand> ]
    // operand:=  <op> | <pre><op> | <op><post>

    return failed_list("parse_tokens_into_expressions() is not implemented yet!");
}

