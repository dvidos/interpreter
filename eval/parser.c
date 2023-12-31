#include <stdbool.h>
#include <stdio.h>
#include "../utils/failable.h"
#include "../utils/list.h"
#include "../utils/stack.h"
#include "parser.h"
#include "token.h"
#include "operator.h"
#include "expression.h"
#include "tokenization.h"

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

enum state { WANT_OPERAND, HAVE_OPERAND, FINISHED };
enum context { NORMAL, PARENTHESIS, FUNC_ARGS, SHORT_IF_TRUE_PART, SHORT_IF_FALSE_PART };

static stack *operators_stack;
static stack *expressions_stack;
static iterator *tokens_iterator;
static token *end_token;
static token *last_token;

static failable_expression parse_expression(token_type expected_finish);




static token* get_token_and_advance() {
    // get token, advance to next position, so we can peek.
    if (!iterator_valid(tokens_iterator)) {
        return end_token;
    }
    token *t = iterator_current(tokens_iterator);
    tokens_iterator = iterator_next(tokens_iterator);
    last_token = t;
    return t;
}

static token* peek_token() {
    // get token, but don't advance to next position
    if (!iterator_valid(tokens_iterator)) {
        return end_token;
    }
    return iterator_current(tokens_iterator);
}

static token* peek_last_token() {
        return last_token;
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

static inline bool is_token_operand(token_type tt) {
    return get_token_operand_operator(tt) != T_UNKNOWN;
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



static failable detect_finish(token_type curr_token, token_type expected_ending, bool *finished) {
    if (expected_ending != T_UNKNOWN) {
        // if there is an expected ending, normal endings are an error
        if (curr_token == T_END || curr_token == T_SEMICOLON)
            return failed("Premature end of tokens, was expecting %d", expected_ending);
        *finished = (curr_token == expected_ending);
    }
    else {
        *finished = (curr_token == T_END || curr_token == T_SEMICOLON);
    }
    return succeeded();
}

static failable parse_expression_on_want_operand(enum state *state) {
    // read a token. If there are no more tokens, announce an error.
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);

    // if the token is an prefix operator or an '(':
    //     mark it as prefix and push it onto the operator stack
    //     goto want_operand
    // if the token is an operand (identifier or variable):
    //     add it to the output queue
    //     goto have_operand
    // if the token is anything else, announce an error and stop. (**)    


    // handle prefix ops 
    if (is_token_prefix_operator(tt)) {
        operator op = get_token_prefix_operator(tt);
        push_operator_for_later(op);
        return succeeded();
    }

    // handle sub-expressions, func cals are handled after having operand.
    // TODO: detect function calls, allow subexpressions to end on a comma or a RPAREN
    if (tt == T_LPAREN) {
        failable_expression sub_expression = parse_expression(T_RPAREN);
        if (sub_expression.failed)
            return failed("%s", sub_expression.err_msg);
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return succeeded();
    }

    // handle operands
    if (is_token_operand(tt)) {
        operator op = get_token_operand_operator(tt);
        expression *e = new_terminal_expression(op, token_get_data(t));
        push_expression(e);
        *state = HAVE_OPERAND;
        return succeeded();
    }

    // nothing else should be expected here
    return failed("Unexpected token type %d, was expecting prefix, operand, or lparen", tt);
}

static failable parse_expression_on_have_operand(enum state *state, token_type expected_ending) {
    // read a token   
    token *t = get_token_and_advance();
    token_type tt = token_get_type(t);

    // if postfix, push for later, and remain in state
    if (is_token_postfix_operator(tt)) {
        operator op = get_token_postfix_operator(tt);
        resolve_pending_higher_precedence_operators(operator_precedence(op));
        push_operator_for_later(op);
        return succeeded();
    }

    // if infix, push it for later, and go back to want-operand
    if (is_token_infix_operator(tt)) {
        operator op = get_token_infix_operator(tt);
        resolve_pending_higher_precedence_operators(operator_precedence(op));
        push_operator_for_later(op);
        *state = WANT_OPERAND;
        return succeeded();
    }

    // detect if we finished (we may be a sub-expression)
    bool finished = false;
    failable fin_detection = detect_finish(tt, expected_ending, &finished);
    if (fin_detection.failed)
        return failed("%s", fin_detection.err_msg);
    if (finished) {
        resolve_pending_higher_precedence_operators(LOWEST_OPERATOR_PRECEDENCE);
        *state = FINISHED;
        return succeeded();
    }
    
    // if the token is a postfix operator:
    //     mark it as postfix and add it to the output queue
    //     goto have_operand.
    // if there are no more tokens (or end-of-expression symbol):
    //     pop all operators off the stack, adding each one to the output queue.
    //     if a `(` is found on the stack, announce an error and stop.
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

static failable_expression parse_expression(token_type expected_finish) {
    // re-entrable, for subexpressions.
    // parse till we meet:
    // - T_END, or, end of expression ';'
    // - possible ternary chars: '?' or ':'
    // - possible RPAREN, for parsing subexpression,
    // - possible comma, for parsing function arguments.

    failable state_handling;
    enum state state = WANT_OPERAND;
    
    while (state != FINISHED) {
        if (token_get_type(peek_token()) == T_END)
            return failed_expression("Premature end of tokens");

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state);
                if (state_handling.failed)
                    return failed_expression("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, expected_finish);
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
    last_token = new_token(T_UNKNOWN);

    // there may be more than one expressions, parse them as long as we have tokens
    list *expressions = new_list();
    while (iterator_valid(tokens_iterator)) {
        failable_expression parsing = parse_expression(T_UNKNOWN);
        if (parsing.failed)
            return failed_list("%s", parsing.err_msg);

        list_add(expressions, parsing.result);
    }

    return ok_list(expressions);
}

// ------------------------------------------------

static bool use_case_passes(const char *code, bool expect_failure, list *expected_expressions) {
    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed) {
        fprintf(stderr, "Code parsing failed unexpectedly (code=\"%s\"): %s", code, tokenization.err_msg);
        return false;
    }

    failable_list parsing = parse_tokens_into_expressions(tokenization.result);

    // test failure
    if (expect_failure) {
        if (!parsing.failed) {
            fprintf(stderr, "Parsing did not fail as expected: (code=\"%s\")\n", code);
            return false;
        }
        return true;
    }

    // success, verify
    if (parsing.failed) {
        fprintf(stderr, "Parsing failed unexpectedly: %s\n\t(code=\"%s\")\n", parsing.err_msg, code);
        return false;
    }

    // compare lengths first
    list *actual_expressions = parsing.result;
    if (list_length(actual_expressions) != list_length(expected_expressions)) {
        fprintf(stderr, "Expected %d expressions, gotten %d, (code=\"%s\")\n", 
            list_length(expected_expressions), list_length(actual_expressions), code);
        expression_print_list(actual_expressions, stderr, "  - ");
        return false;
    }

    // compare each expression
    for (int i = 0; i < list_length(expected_expressions); i++) {
        expression *actual = list_get(expected_expressions, i);
        expression *expected = list_get(actual_expressions, i);
        if (!expressions_are_equal(actual, expected)) {
            fprintf(stderr, "Expression #%d differs, (code=\"%s\"), expected expression:\n", i, code);
            expression_print(expected, stderr, "  ");
            fprintf(stderr, "actual:\n");
            expression_print(actual, stderr, "  ");
            return false;
        }
    }

    return true;
}

// perform unit tests and report if successful
bool parser_self_diagnostics() {
    bool all_passed = true;

    if (!use_case_passes(NULL, false, list_of(0)))
        all_passed = false;

    if (!use_case_passes("", false, list_of(0)))
        all_passed = false;

    if (!use_case_passes("a+1", false, list_of(1,
        new_binary_expression(OP_ADDITION, 
            new_terminal_expression(OP_SYMBOL_VALUE, "a"),
            new_terminal_expression(OP_NUMBER_VALUE, "1")
        )
    ))) all_passed = false;

    return all_passed;
}

