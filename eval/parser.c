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

typedef enum run_state { WANT_OPERAND, HAVE_OPERAND, FINISHED } run_state;
typedef enum run_context { CTX_NORMAL, CTX_PARENTHESIS, FUNC_ARGS, SHORT_IF_TRUE_PART, SHORT_IF_FALSE_PART } run_context;


static stack *operators_stack;
static stack *expressions_stack;
static sequential *tokens_sequential;
static token *last_token;
static token *end_token;
static failable_expression parse_expression(run_context context);


static token* get_token_and_advance() {
    // get token, advance to next position, so we can peek.
    if (tokens_sequential == NULL)
        return end_token;
    
    token *t = (token *)tokens_sequential->data;
    tokens_sequential = tokens_sequential->next;
    last_token = t;
    return t;
}

static token* peek_token() {
    // get token, but don't advance to next position
    if (tokens_sequential == NULL)
        return end_token;
    
    return (token *)tokens_sequential->data;
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

static void print_operators_stack(FILE *stream, char *prefix) {
    fprintf(stream, "%sOperators stack (%d)\n", prefix, stack_length(operators_stack));
    sequential *s = stack_sequential(operators_stack);
    while (s != NULL) {
        fprintf(stream, "%s  - %s\n", prefix, operator_str((operator)s->data));
        s = s->next;
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

static void print_expression_stack(FILE *stream, char *prefix) {
    fprintf(stream, "%sExpressions stack (%d)\n", prefix, stack_length(expressions_stack));
    sequential *s = stack_sequential(expressions_stack);
    if (s == NULL) {
        fprintf(stream, "%s  (empty)\n", prefix);
    } else {
        while (s != NULL) {
            fprintf(stream, "%s", prefix);
            expression_print((expression *)s->data, stream, "  - ", true);
            s = s->next;
        }
    }
}

// --------------------------------------------

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
    while (operator_precedence(top_op) < precedence) {
        make_one_expression_from_top_operator();
        top_op = peek_top_operator();
    }
}

// --------------------------------------------

static failable detect_finish(token_type curr_token, run_context context, bool *finished) {
    if (context == CTX_NORMAL) {
        *finished = (curr_token == T_END || curr_token == T_SEMICOLON);

    } else if (context == CTX_PARENTHESIS) {
        if (curr_token == T_END || curr_token == T_SEMICOLON)
            return failed("Premature end of tokens, was expecting ')' to end parenthesis context");
        *finished = (curr_token == T_RPAREN);
    }
    return succeeded();
}

static failable parse_expression_on_want_operand(run_state *state) {
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
    if (tt == T_LPAREN) {
        failable_expression sub_expression = parse_expression(CTX_PARENTHESIS);
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
    return failed("Unexpected token type %s, was expecting prefix, operand, or lparen", token_type_str(tt));
}

static failable parse_expression_on_have_operand(run_state *state, run_context context) {
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
    failable fin_detection = detect_finish(tt, context, &finished);
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

        print_expression_stack(stderr, "    ");
        print_operators_stack(stderr, "    ");
}

static failable_expression parse_expression(run_context context) {
    // re-entrable, for subexpressions.
    // parse till we meet:
    // - T_END, or, end of expression ';'
    // - possible ternary chars: '?' or ':'
    // - possible RPAREN, for parsing subexpression,
    // - possible comma, for parsing function arguments.

    failable state_handling;
    run_state state = WANT_OPERAND;
    push_operator_for_later(OP_SENTINEL);

    while (state != FINISHED) {
        // print_debug_information("parse_expression() loop", state);

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state);
                if (state_handling.failed)
                    return failed_expression("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, context);
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
    tokens_sequential = list_sequential(tokens);
    end_token = new_token(T_END);
    last_token = new_token(T_UNKNOWN);

    // there may be more than one expressions, parse them as long as we have tokens
    list *expressions = new_list();
    while (tokens_sequential != NULL && token_get_type(peek_token()) != T_END) {
        failable_expression parsing = parse_expression(CTX_NORMAL);
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
        fprintf(stderr, "Parsing tokenization failed unexpectedly: %s\n\t(code=\"%s\")", tokenization.err_msg, code);
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
        expression_print_list(actual_expressions, stderr, "  - ", false);
        return false;
    }

    // compare each expression
    for (int i = 0; i < list_length(expected_expressions); i++) {
        expression *actual = list_get(actual_expressions, i);
        expression *expected = list_get(expected_expressions, i);
        if (!expressions_are_equal(actual, expected)) {
            fprintf(stderr, "Expression #%d differs, (code=\"%s\"), \n  expected: ", i, code);
            expression_print(expected, stderr, "", false);
            fprintf(stderr, "  actual:   ");
            expression_print(actual, stderr, "", false);
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

