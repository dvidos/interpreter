#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/str_builder.h"
#include "../../utils/failable.h"
#include "../../utils/containers/_module.h"
#include "expression_parser.h"
#include "statement_parser.h"
#include "../lexer/_module.h"


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


static stack *operators_stack;
static stack *expressions_stack;
static iterator *tokens_iterator;
static token *prev_token;
static token *end_token;
static token *_last_accepted = NULL;

void initialize_expression_parser() {
    operators_stack = new_stack(containing_operators);
    expressions_stack = new_stack(containing_expressions);
    end_token = new_token(T_END, NULL, 0, 0);
    prev_token = new_token(T_UNKNOWN, NULL, 0, 0);
}

static bool accept(token_type tt) {
    token *t = tokens_iterator->curr(tokens_iterator);
    if (token_get_type(t) != tt)
        return false;
    _last_accepted = t;
    prev_token = t;
    tokens_iterator->next(tokens_iterator);
    return true;
}
static inline token *last_accepted() {
    return _last_accepted;
}
static token* get_token_and_advance() {
    // get token, advance to next position, so we can peek.
    if (!tokens_iterator->valid(tokens_iterator))
        return end_token;
    
    prev_token = tokens_iterator->curr(tokens_iterator);
    tokens_iterator->next(tokens_iterator);
    return prev_token;
}
static token* peek() {
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
    const char *s = stack_to_string(operators_stack, ", ");
    fprintf(stream, "%sOperators   stack, %d items, top -> %s <- bottom\n", 
        prefix, stack_length(operators_stack), s);
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
    const char *s = stack_to_string(expressions_stack, ", ");
    fprintf(stream, "%sExpressions stack, %d items, top -> %s <- bottom\n", 
        prefix, stack_length(expressions_stack), s);
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

static void create_expressions_for_higher_operators_than(operator new_op) {
    // the operators stack always has the highest precedence ops at the top.
    // if we want to add a smaller precedence, we pop them into expressions
    // this assumes the use of the SENTINEL, the lowest priority operator
    // left-associated operators allow equal precedence to be popped,
    // so that 8-4-2 => (8-4)-2 and not 8-(4-2).
    int new_precedence = operator_precedence(new_op);
    while (true) {
        operator top_op = peek_top_operator();
        int top_precedence = operator_precedence(top_op);
        bool top_is_unary = operator_is_unary(top_op);
        op_associativity top_assoc = operator_associativity(top_op);

        bool top_is_higher;
        if (top_op == OP_SENTINEL)
            top_is_higher = false;
        else if (top_is_unary)
            top_is_higher = top_precedence <= new_precedence;
        else
            top_is_higher =
                (top_precedence < new_precedence) ||
                (top_assoc == L2R && top_precedence == new_precedence);
        
        if (!top_is_higher)
            break;
        
        make_one_expression_from_top_operator();
    }
}

// --------------------------------------------

static failable_bool detect_completion(token_type curr_token, completion_mode completion) {
    if (completion == CM_END_OF_TEXT) {
        // we can accept END here.
        return ok_bool(curr_token == T_END);
    } else if (completion == CM_SEMICOLON_OR_END){
        return ok_bool(curr_token == T_SEMICOLON || curr_token == T_END);
    }

    // we do not accept END from here onwards
    if (curr_token == T_END)
        return failed_bool("Unexpected end of expression (%s), when completion mode is %d", 
                token_type_str(curr_token), completion);
    
    if (completion == CM_SEMICOLON) {
        return ok_bool(curr_token == T_SEMICOLON);
    } else if (completion == CM_RPAREN) {
        return ok_bool(curr_token == T_RPAREN);
    } else if (completion == CM_COMMA_OR_RPAREN) {
        return ok_bool(curr_token == T_COMMA || curr_token == T_RPAREN);
    } else if (completion == CM_COMMA_OR_RSQBRACKET) {
        return ok_bool(curr_token == T_COMMA || curr_token == T_RSQBRACKET);
    } else if (completion == CM_COMMA_OR_RBRACKET) {
        return ok_bool(curr_token == T_COMMA || curr_token == T_RBRACKET);
    } else if (completion == CM_COLON) {
        return ok_bool(curr_token == T_COLON);
    }

    return failed_bool("Unknown completion mode %d", completion);
}

static failable_expression parse_list_initializer(bool verbose) {
    list *l = new_list(containing_expressions);

    // [] = empty list
    if (accept(T_RSQBRACKET))
        return ok_expression(new_list_data_expression(l));

    // else parse expressions until we reach end square bracket.
    while (token_get_type(prev_token) != T_RSQBRACKET) {
        failable_expression expr = parse_expression(tokens_iterator, CM_COMMA_OR_RSQBRACKET, verbose);
        if (expr.failed) return failed_expression("%s", expr.err_msg);
        list_add(l, expr.result);
    }

    return ok_expression(new_list_data_expression(l));
}

static failable_expression parse_dict_initializer(bool verbose) {
    dict *d = new_dict(containing_expressions, 64);

    // {} = empty dict
    if (accept(T_RBRACKET))
        return ok_expression(new_dict_data_expression(d));

    // else parse "key":expression until we reach end square bracket.
    while (token_get_type(prev_token) != T_RBRACKET) {
        failable_expression key_expr = parse_expression(tokens_iterator, CM_COLON, verbose);
        if (key_expr.failed) return failed_expression("%s", key_expr.err_msg);
        if (expression_get_type(key_expr.result) != ET_IDENTIFIER)
            return failed_expression("Dict keys should be identifiers, got %d", expression_get_type(key_expr.result));
        const char *key = expression_get_terminal_data(key_expr.result);
        failable_expression val_expr = parse_expression(tokens_iterator, CM_COMMA_OR_RBRACKET, verbose);
        if (val_expr.failed) return failed_expression("%s", val_expr.err_msg);
        dict_set(d, key, val_expr.result);
    }

    return ok_expression(new_dict_data_expression(d));
}

static failable_expression parse_func_declaration_expression(bool verbose) {
    // past 'function', expected: "( [args] ) { [statements] }"
    if (!accept(T_LPAREN))
        return failed_expression("Expected '(' after function");
    
    list *arg_names = new_list(containing_strs);
    while (!accept(T_RPAREN)) {
        if (!accept(T_IDENTIFIER))
            return failed_expression("Expected identifier in function arg names");
        list_add(arg_names, (void *)token_get_data(last_accepted())); // we lose const here
        accept(T_COMMA);
    }

    failable_list statements_parsing = parse_statements(tokens_iterator, SP_BLOCK_MANDATORY);
    if (statements_parsing.failed) return failed_expression("Failed parsing function body: %s", statements_parsing.err_msg);

    return ok_expression(new_func_decl_expression(arg_names, statements_parsing.result));
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

    // handle sub-expressions here, func cals are handled after having operand.
    if (tt == T_LPAREN) {
        failable_expression sub_expression = parse_expression(tokens_iterator, CM_RPAREN, verbose);
        if (sub_expression.failed) return failed("Subexpression failed: %s", sub_expression.err_msg);
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    if (tt == T_LSQBRACKET) {
        failable_expression list_expression = parse_list_initializer(verbose);
        if (list_expression.failed) return failed("List initialization failed: %s", list_expression.err_msg);
        push_expression(list_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    if (tt == T_LBRACKET) {
        failable_expression dict_expression = parse_dict_initializer(verbose);
        if (dict_expression.failed) return failed("Dict initialization failed: %s", dict_expression.err_msg);
        push_expression(dict_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    if (tt == T_FUNCTION) {
        // parse an inline function declaration
        failable_expression func_expression = parse_func_declaration_expression(verbose);
        if (func_expression.failed) return failed("Parsing func declaration failed: %s", func_expression.err_msg);
        push_expression(func_expression.result);
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

static failable_list parse_function_call_arguments_expressions(bool verbose) {
    list *args = new_list(containing_expressions);

    // if empty args, there will be nothing to parse
    if (token_get_type(peek()) == T_RPAREN) {
        get_token_and_advance();
        return ok_list(args);
    }

    while (token_get_type(prev_token) != T_RPAREN) {
        failable_expression parse_arg = parse_expression(tokens_iterator, CM_COMMA_OR_RPAREN, verbose);
        if (parse_arg.failed)
            return failed_list("%s", parse_arg.err_msg);
        list_add(args, parse_arg.result);
    }
    
    return ok_list(args);
}

static failable_expression parse_shorthand_if_pair(bool verbose) {
    failable_expression parsing = parse_expression(tokens_iterator, CM_COLON, verbose);
    if (parsing.failed) return failed_expression("%s", parsing.err_msg);
    expression *e1 = parsing.result;

    parsing = parse_expression(tokens_iterator, CM_END_OF_TEXT, verbose);
    if (parsing.failed) return failed_expression("%s", parsing.err_msg);
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
        failable_list arg_expressions = parse_function_call_arguments_expressions(verbose);
        if (arg_expressions.failed)
            return failed("%s", arg_expressions.err_msg);
        push_expression(new_list_data_expression(arg_expressions.result));
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
        // here, if same operation precedence as top of stack, 
        // maybe pop one and make one.
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
        if (prev_token == NULL)
            fprintf(stderr, "(null)");
        else
            token_print(prev_token, stderr, "");

        fprintf(stderr, ", next token ");
        token_print(peek(), stderr, "");
        fprintf(stderr, "\n");

        print_expressions_stack(stderr, "    ");
        print_operators_stack(stderr, "    ");
}

failable_expression parse_expression(iterator *tokens, completion_mode completion, bool verbose) {
    // re-entrable
    tokens_iterator = tokens;

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
                    return failed_expression("Failed on want operand: %s", state_handling.err_msg);
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, completion, verbose);
                if (state_handling.failed)
                    return failed_expression("Failed on have operand: %s", state_handling.err_msg);
                break;
        }

        if (verbose)
            print_debug_information("after one instanceparse_expression() loop", state);
    }

    if (peek_top_operator() != OP_SENTINEL)
        return failed_expression("Was expecting SENTINEL at the top of the queue");
    pop_top_operator();

    return ok_expression(pop_top_expression());
}