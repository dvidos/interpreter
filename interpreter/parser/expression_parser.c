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
static token *end_token;
static token *_last_accepted_token = NULL;

void initialize_expression_parser() {
    operators_stack = new_stack(containing_operators);
    expressions_stack = new_stack(containing_expressions);
    end_token = new_token(T_END, NULL, 0, 0);
}

static bool accept(token_type tt) {
    token *t = tokens_iterator->curr(tokens_iterator);
    if (token_get_type(t) != tt)
        return false;
    _last_accepted_token = t;
    if (token_get_type(t) != T_END)
        tokens_iterator->next(tokens_iterator);
    return true;
}
static inline token *accepted() {
    return _last_accepted_token;
}
static token* peek() {
    // get token, but don't advance to next position
    if (!tokens_iterator->valid(tokens_iterator))
        return end_token;
    
    return tokens_iterator->curr(tokens_iterator);
}

static inline operator make_positioned_operator(token *t, op_position position) {
    return operator_by_type_and_position(token_get_type(t), position);
}

static inline bool accept_positioned_operator(op_position position) {
    token *t = peek();
    operator possible = make_positioned_operator(t, position);
    if (possible == T_UNKNOWN)
        return false; // not an operator at this position
    
    return accept(token_get_type(t));
}

static inline bool accept_operand() {
    return accept(T_IDENTIFIER) ||
           accept(T_NUMBER_LITERAL) ||
           accept(T_STRING_LITERAL) ||
           accept(T_BOOLEAN_LITERAL);
}

static inline expression *make_operand_expression(token *t) {
    const char *data = token_get_data(t);
    switch (token_get_type(t)) {
        case T_IDENTIFIER: return new_identifier_expression(data);
        case T_NUMBER_LITERAL: return new_numeric_literal_expression(data);
        case T_STRING_LITERAL: return new_string_literal_expression(data);
        case T_BOOLEAN_LITERAL: return new_boolean_literal_expression(data);
    }
    return NULL;
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

static failable_bool detect_completion(completion_mode mode) {

    if (mode == CM_END_OF_TEXT) {
        // we can accept END here.
        return ok_bool(accept(T_END));
    } else if (mode == CM_SEMICOLON_OR_END){
        return ok_bool(accept(T_SEMICOLON) || accept(T_END));
    }

    // we do not accept END from here onwards
    if (accept(T_END))
        return failed_bool("Unexpected end of expression, when completion mode is %d", mode);
    
    if      (mode == CM_SEMICOLON)           return ok_bool(accept(T_SEMICOLON));
    else if (mode == CM_COLON)               return ok_bool(accept(T_COLON));
    else if (mode == CM_RPAREN)              return ok_bool(accept(T_RPAREN));
    else if (mode == CM_RSQBRACKET)          return ok_bool(accept(T_RSQBRACKET));
    else if (mode == CM_COMMA_OR_RPAREN)     return ok_bool(accept(T_COMMA) || accept(T_RPAREN));
    else if (mode == CM_COMMA_OR_RSQBRACKET) return ok_bool(accept(T_COMMA) || accept(T_RSQBRACKET));
    else if (mode == CM_COMMA_OR_RBRACKET)   return ok_bool(accept(T_COMMA) || accept(T_RBRACKET));

    return failed_bool("Unknown completion mode %d", mode);
}

static failable_expression parse_list_initializer(bool verbose) {
    list *l = new_list(containing_expressions);

    // [] = empty list
    if (accept(T_RSQBRACKET))
        return ok_expression(new_list_data_expression(l));

    // else parse expressions until we reach end square bracket.
    while (token_get_type(accepted()) != T_RSQBRACKET) {
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
    while (token_get_type(accepted()) != T_RBRACKET) {
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
        list_add(arg_names, (void *)token_get_data(accepted())); // we lose const here
        accept(T_COMMA);
    }

    failable_list statements_parsing = parse_statements(tokens_iterator, SP_BLOCK_MANDATORY);
    if (statements_parsing.failed) return failed_expression("Failed parsing function body: %s", statements_parsing.err_msg);

    return ok_expression(new_func_decl_expression(arg_names, statements_parsing.result));
}

static failable parse_expression_on_want_operand(run_state *state, bool verbose) {

    // prefix operators come before the operand
    if (accept_positioned_operator(PREFIX)) {
        push_operator_for_later(make_positioned_operator(accepted(), PREFIX));
        return ok();
    }

    // a parenthesis is a sub-expression here, func cals are handled after having operand.
    if (accept(T_LPAREN)) {
        failable_expression sub_expression = parse_expression(tokens_iterator, CM_RPAREN, verbose);
        if (sub_expression.failed) return failed("Subexpression failed: %s", sub_expression.err_msg);
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "nums = [ 1, 2, 3, 5, 8, 13 ]"
    if (accept(T_LSQBRACKET)) {
        failable_expression list_expression = parse_list_initializer(verbose);
        if (list_expression.failed) return failed("List initialization failed: %s", list_expression.err_msg);
        push_expression(list_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "person = { name: "john", age: 30 };"
    if (accept(T_LBRACKET)) {
        failable_expression dict_expression = parse_dict_initializer(verbose);
        if (dict_expression.failed) return failed("Dict initialization failed: %s", dict_expression.err_msg);
        push_expression(dict_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "pie = function() { return 3.14; }"
    if (accept(T_FUNCTION_KEYWORD)) {
        failable_expression func_expression = parse_func_declaration_expression(verbose);
        if (func_expression.failed) return failed("Parsing func declaration failed: %s", func_expression.err_msg);
        push_expression(func_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    if (accept_operand()) {
        push_expression(make_operand_expression(accepted()));
        *state = HAVE_OPERAND;
        return ok();
    }

    // nothing else should be expected here
    return failed("Unexpected token type %s, was expecting prefix, operand, or lparen", token_type_str(token_get_type(peek())));
}

static failable_list parse_function_call_arguments_expressions(bool verbose) {
    list *args = new_list(containing_expressions);

    // if empty args, there will be nothing to parse
    if (accept(T_RPAREN))
        return ok_list(args);

    while (token_get_type(accepted()) != T_RPAREN) {
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

    // if postfix, push for later, and remain in state
    if (accept_positioned_operator(POSTFIX)) {
        operator op = make_positioned_operator(accepted(), POSTFIX);
        create_expressions_for_higher_operators_than(op);
        push_operator_for_later(op);
        return ok();
    }

    // special parsing of arguments after a function call parenthesis
    if (accept(T_LPAREN)) {
        failable_list arg_expressions = parse_function_call_arguments_expressions(verbose);
        if (arg_expressions.failed)
            return failed("%s", arg_expressions.err_msg);
        create_expressions_for_higher_operators_than(OP_FUNC_CALL);
        push_operator_for_later(OP_FUNC_CALL);
        push_expression(new_list_data_expression(arg_expressions.result));
        *state = HAVE_OPERAND;
        return ok();
    }

    if (accept(T_QUESTION_MARK)) {
        failable_expression if_parts = parse_shorthand_if_pair(verbose);
        if (if_parts.failed) return failed("%s", if_parts.err_msg);
        create_expressions_for_higher_operators_than(OP_SHORT_IF);
        push_operator_for_later(OP_SHORT_IF);
        push_expression(if_parts.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // an array subscript, we must parse the ']'
    if (accept(T_LSQBRACKET)) {
        failable_expression subscript = parse_expression(tokens_iterator, CM_RSQBRACKET, verbose);
        if (subscript.failed) return failed("%s", subscript.err_msg);
        create_expressions_for_higher_operators_than(OP_ARRAY_SUBSCRIPT);
        push_operator_for_later(OP_ARRAY_SUBSCRIPT);
        push_expression(subscript.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // if infix, push it for resolving later, and go back to want-operand
    if (accept_positioned_operator(INFIX)) {
        operator op = make_positioned_operator(accepted(), INFIX);
        create_expressions_for_higher_operators_than(op);
        push_operator_for_later(op);
        *state = WANT_OPERAND;
        return ok();
    }

    // detect if we finished (we may be a sub-expression)
    failable_bool completion_detection = detect_completion(completion);
    if (completion_detection.failed) return failed("%s", completion_detection.err_msg);
    if (completion_detection.result) {
        create_expressions_for_higher_operators_than(OP_SENTINEL);
        *state = FINISHED;
        return ok();
    }
    
    // return ok();
    return failed("have operand ran out of options??? peek() -> %s", token_type_str(token_get_type(peek())));
}

static void print_debug_information(char *title, run_state state) {
        fprintf(stderr, "%s\n", title);

        char *state_name;
        switch (state) {
            case WANT_OPERAND: state_name = "WANT_OPERAND"; break;
            case HAVE_OPERAND: state_name = "HAVE_OPERAND"; break;
            case FINISHED:     state_name = "FINISHED"; break;
            default:           state_name = "(unknown)";
        }
        fprintf(stderr, "    state=%s, accepted()=%s, peek()=%s\n",
            state_name,
            accepted() == NULL ? "(null)" : token_type_str(token_get_type(accepted())),
            token_type_str(token_get_type(peek()))
        );

        print_expressions_stack(stderr, "    ");
        print_operators_stack(stderr, "    ");
}

failable_expression parse_expression(iterator *tokens, completion_mode completion, bool verbose) {
    // re-entrable
    tokens_iterator = tokens;
    run_state state = WANT_OPERAND;

    if (verbose)
        print_debug_information("parse_expression() starting", state);
        
    failable state_handling;
    push_operator_for_later(OP_SENTINEL);

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
            print_debug_information("parse_expression() step", state);
    }

    operator sentinel = pop_top_operator();
    if (sentinel != OP_SENTINEL)
        return failed_expression("Was expecting SENTINEL at the top of the queue");

    expression *result = pop_top_expression();

    if (verbose)
        print_debug_information("parse_expression() ended", state);
    
    return ok_expression(result);
}
