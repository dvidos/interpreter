#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/strbuff.h"
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
typedef enum completion_mode { CM_NORMAL, CM_SUB_EXPRESSION, CM_FUNC_ARGS } completion_mode;


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
        strbuff *separator_sb = new_strbuff();
        strbuff_catf(separator_sb, "\n%s    ", prefix);
        const char *str = stack_to_string(expressions_stack, strbuff_charptr(separator_sb));
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

static failable_bool detect_completion(token_type curr_token, completion_mode completion) {
    if (completion == CM_NORMAL) {
        return ok_bool(curr_token == T_END || curr_token == T_SEMICOLON);
    }

    // all other modes should not end prematurely.
    if (curr_token == T_END || curr_token == T_SEMICOLON)
        return failed_bool("Unexpected end of expression (%s), when completion mode is %d", 
                token_type_str(curr_token), completion);
    
    if (completion == CM_SUB_EXPRESSION) {
        return ok_bool(curr_token == T_RPAREN);
    } else if (completion == CM_FUNC_ARGS) {
        return ok_bool(curr_token == T_RPAREN || curr_token == T_COMMA);
    }

    return failed_bool("Unknown completion mode %d", completion);
}

static failable parse_expression_on_want_operand(run_state *state, bool verbose) {
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
        failable_expression sub_expression = parse_expression(CM_SUB_EXPRESSION, verbose);
        if (sub_expression.failed)
            return failed("Subexpression failed: %s", sub_expression.err_msg);
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return succeeded();
    }

    // handle operands
    if (is_token_operand(tt)) {
        expression *e = get_operand_expression(tt, token_get_data(t));
        push_expression(e);
        *state = HAVE_OPERAND;
        return succeeded();
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
            return failed_list("%s", parse_arg.err_msg);
        list_add(args, parse_arg.result);
    }
    
    return ok_list(args);
}

static failable parse_expression_on_have_operand(run_state *state, completion_mode context, bool verbose) {
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

    // handle function calls, operand is function name or address
    if (tt == T_LPAREN) {
        failable_list arg_expressions = parse_function_arguments_expressions(verbose);
        if (arg_expressions.failed)
            return failed("%s", arg_expressions.err_msg);
        push_expression(new_func_args_expression(arg_expressions.result));

        resolve_pending_higher_precedence_operators(operator_precedence(OP_FUNC_CALL));
        push_operator_for_later(OP_FUNC_CALL);
        *state = HAVE_OPERAND;
        return succeeded();
    }

    // if infix, push it for resolving later, and go back to want-operand
    if (is_token_infix_operator(tt)) {
        operator op = get_token_infix_operator(tt);
        resolve_pending_higher_precedence_operators(operator_precedence(op));
        push_operator_for_later(op);
        *state = WANT_OPERAND;
        return succeeded();
    }

    // detect if we finished (we may be a sub-expression)
    failable_bool completion_detection = detect_completion(tt, context);
    if (completion_detection.failed)
        return failed("%s", completion_detection.err_msg);
    if (completion_detection.result) {
        resolve_pending_higher_precedence_operators(operator_precedence(OP_SENTINEL));
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

        print_expressions_stack(stderr, "    ");
        print_operators_stack(stderr, "    ");
}

static failable_expression parse_expression(completion_mode context, bool verbose) {
    // re-entrable, for subexpressions.
    // parse till we meet:
    // - T_END, or, end of expression ';'
    // - possible ternary chars: '?' or ':'
    // - possible RPAREN, for parsing subexpression,
    // - possible comma, for parsing function arguments.

    failable state_handling;
    run_state state = WANT_OPERAND;
    push_operator_for_later(OP_SENTINEL);

    if (verbose)
        print_debug_information("parse_expression() loop", state);
    while (state != FINISHED) {

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state, verbose);
                if (state_handling.failed)
                    return failed_expression("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, context, verbose);
                if (state_handling.failed)
                    return failed_expression("Failed on have operand: %s", state_handling.err_msg);
                break;
        }

        if (verbose)
            print_debug_information("parse_expression() loop", state);
    }

    if (peek_top_operator() != OP_SENTINEL)
        return failed_expression("Was expecting SENTINEL at the top of the queue");
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
            return failed_list("%s", parsing.err_msg);

        list_add(expressions, parsing.result);
    }

    return ok_list(expressions);
}

// ------------------------------------------------

static bool use_case_passes(const char *code, bool expect_failure, list *expected_expressions, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed) {
        fprintf(stderr, "Parsing tokenization failed unexpectedly: %s\n\t(code=\"%s\")", tokenization.err_msg, code);
        return false;
    }

    failable_list parsing = parse_tokens_into_expressions(tokenization.result, verbose);

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
    list *parsed_expressions = parsing.result;
    if (list_length(parsed_expressions) != list_length(expected_expressions)) {
        fprintf(stderr, "Expected %d expressions, gotten %d, (code=\"%s\")\n", 
            list_length(expected_expressions), list_length(parsed_expressions), code);
        fprintf(stderr, "    %s\n", list_to_string(parsed_expressions, "\n    "));
        return false;
    }

    // compare each expression
    for (int i = 0; i < list_length(expected_expressions); i++) {
        expression *parsed = list_get(parsed_expressions, i);
        expression *expected = list_get(expected_expressions, i);
        if (!expressions_are_equal(parsed, expected)) {
            fprintf(stderr, "Expression #%d differs, (code=\"%s\"), \n" \
                            "  expected: %s\n" \
                            "  parsed:   %s\n",
                            i, code, expression_to_string(expected), expression_to_string(parsed));
            return false;
        }
    }

    return true;
}

// perform unit tests and report if successful
bool parser_self_diagnostics(bool verbose) {
    bool all_passed = true;

    if (!use_case_passes(NULL, false, list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("",   false, list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("+",  true,  list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("a+", true,  list_of(0), verbose)) all_passed = false;

    if (!use_case_passes("a+1", false, list_of(1,
        new_binary_op_expression(OP_ADD, 
            new_identifier_expression("a"),
            new_numeric_literal_expression("1")
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("1+2*3+4", false, list_of(1,
        new_binary_op_expression(OP_ADD, 
            new_numeric_literal_expression("1"),
            new_binary_op_expression(OP_ADD, 
                new_binary_op_expression(OP_MULTIPLY, 
                    new_numeric_literal_expression("2"),
                    new_numeric_literal_expression("3")
                ),
                new_numeric_literal_expression("4")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("(1+2)*(3+4)", false, list_of(1,
        new_binary_op_expression(OP_MULTIPLY, 
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("1"),
                new_numeric_literal_expression("2")
            ),
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("3"),
                new_numeric_literal_expression("4")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("time(",    true, list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("time(,",   true, list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("time(1,",  true, list_of(0), verbose)) all_passed = false;
    if (!use_case_passes("time(1,2", true, list_of(0), verbose)) all_passed = false;

    if (!use_case_passes("time()", false, list_of(1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("time"),
            new_func_args_expression(list_of(0))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14)", false, list_of(1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(1,
                new_numeric_literal_expression("3.14")
            ))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14, 2)", false, list_of(1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(2,
                new_numeric_literal_expression("3.14"),
                new_numeric_literal_expression("2")
            ))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("pow(8, 2) + 1", false, list_of(1,
        new_binary_op_expression(OP_ADD,
            new_binary_op_expression(OP_FUNC_CALL,
                new_identifier_expression("pow"),
                new_func_args_expression(list_of(2,
                    new_numeric_literal_expression("8"),
                    new_numeric_literal_expression("2")
                ))
            ),
            new_numeric_literal_expression("1")
    )), verbose)) all_passed = false;

    if (!use_case_passes("a == 0", false, list_of(1,
        new_binary_op_expression(OP_EQUAL,
            new_identifier_expression("a"),
            new_numeric_literal_expression("0")
        )
    ), verbose)) all_passed = false;
    
    if (!use_case_passes("if(left(a, 1) == '0', 'number', 'letter')", false, list_of(1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("if"),
            new_func_args_expression(list_of(3, 
                new_binary_op_expression(OP_EQUAL, 
                    new_binary_op_expression(OP_FUNC_CALL,
                        new_identifier_expression("left"),
                        new_func_args_expression(list_of(2, 
                            new_identifier_expression("a"),
                            new_numeric_literal_expression("1")
                        ))),
                    new_string_literal_expression("0")),
                new_string_literal_expression("number"),
                new_string_literal_expression("letter")
            ))
        )
    ), verbose)) all_passed = false;
    
    return all_passed;
}
