#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/str.h"
#include "../utils/str_builder.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
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
static token *last_accepted_token = NULL;

void initialize_expression_parser() {
    operators_stack = new_stack(pair_class);
    expressions_stack = new_stack(expression_class);
    last_accepted_token = NULL;
}

static bool accept(token_type tt) {
    token *t = tokens_iterator->curr(tokens_iterator);
    if (token_get_type(t) != tt)
        return false;
    last_accepted_token = t;
    if (token_get_type(t) != T_END)
        tokens_iterator->next(tokens_iterator);
    return true;
}

static inline token *accepted() {
    return last_accepted_token;
}

static token* peek() {
    return tokens_iterator->curr(tokens_iterator);
}

static inline operator_type make_positioned_operator(token *t, op_type_position position) {
    return operator_type_by_token_and_position(token_get_type(t), position);
}

static inline bool accept_positioned_operator(op_type_position position) {
    token *t = peek();
    operator_type possible = make_positioned_operator(t, position);
    if (possible == T_UNKNOWN)
        return false; // not an operator_type at this position
    
    return accept(token_get_type(t));
}

static inline bool accept_operand() {
    return accept(T_IDENTIFIER) ||
           accept(T_NUMBER_LITERAL) ||
           accept(T_STRING_LITERAL) ||
           accept(T_BOOLEAN_LITERAL);
}

static inline expression *make_operand_expression(token *token) {
    const char *data = token_get_data(token);
    switch (token_get_type(token)) {
        case T_IDENTIFIER: return new_identifier_expression(data, token);
        case T_NUMBER_LITERAL: return new_numeric_literal_expression(data, token);
        case T_STRING_LITERAL: return new_string_literal_expression(data, token);
        case T_BOOLEAN_LITERAL: return new_boolean_literal_expression(data, token);
    }
    return NULL;
}

static inline void push_operator_pair(operator_type op, token *token) {
    stack_push(operators_stack, new_pair(
        operator_type_class, (void *)op, 
        token_class, token
    ));
}

static inline pair *peek_top_operator_pair() {
    return stack_peek(operators_stack);
}

static inline pair *pop_top_operator_pair() {
    return stack_pop(operators_stack);
}

static void print_operators_stack(FILE *stream, char *prefix) {
    str_builder *sb = new_str_builder();
    stack_describe(operators_stack, ", ", sb);
    fprintf(stream, "%sOperators   stack, %d items, top -> %s <- bottom\n", 
        prefix, stack_length(operators_stack), str_builder_charptr(sb));
    str_builder_free(sb);
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
    str_builder *sb = new_str_builder();
    stack_describe(expressions_stack, ", ", sb);
    fprintf(stream, "%sExpressions stack, %d items, top -> %s <- bottom\n", 
        prefix, stack_length(expressions_stack), str_builder_charptr(sb));
    str_builder_free(sb);
}

// --------------------------------------------

static void make_one_expression_from_top_operator() {
    pair *p = pop_top_operator_pair();
    operator_type op_type = (operator_type)pair_get_left(p);
    token *token = pair_get_right(p);
    op_type_position pos = operator_type_position(op_type);
    expression *new_expr;

    if (pos == PREFIX || pos == POSTFIX) {
        expression *operand1 = pop_top_expression();
        new_expr = new_unary_expression(op_type, token, operand1);
    } else if (pos == INFIX) {
        // note that we pop the second first, as it was pushed last
        expression *operand2 = pop_top_expression();
        expression *operand1 = pop_top_expression();
        new_expr = new_binary_expression(op_type, token, operand1, operand2);
    }
    
    push_expression(new_expr);
}

static void create_expressions_for_higher_operators_than(operator_type new_op) {
    // the operators stack always has the highest precedence ops at the top.
    // if we want to add a smaller precedence, we pop them into expressions
    // this assumes the use of the SENTINEL, the lowest priority operator_type
    // left-associated operators allow equal precedence to be popped,
    // so that 8-4-2 => (8-4)-2 and not 8-(4-2).
    int new_precedence = operator_type_precedence(new_op);
    while (true) {
        pair *top_pair = peek_top_operator_pair();
        operator_type top_type = (operator_type)pair_get_left(top_pair);
        int top_precedence = operator_type_precedence(top_type);
        bool top_is_unary = operator_type_is_unary(top_type);
        op_type_associativity top_assoc = operator_type_associativity(top_type);

        bool top_is_higher;
        if (top_type == OP_SENTINEL)
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
        return failed_bool(NULL, "Unexpected end of expression, when completion mode is %d", mode);
    
    if      (mode == CM_SEMICOLON)           return ok_bool(accept(T_SEMICOLON));
    else if (mode == CM_COLON)               return ok_bool(accept(T_COLON));
    else if (mode == CM_RPAREN)              return ok_bool(accept(T_RPAREN));
    else if (mode == CM_RSQBRACKET)          return ok_bool(accept(T_RSQBRACKET));
    else if (mode == CM_COMMA_OR_RPAREN)     return ok_bool(accept(T_COMMA) || accept(T_RPAREN));
    else if (mode == CM_COMMA_OR_RSQBRACKET) return ok_bool(accept(T_COMMA) || accept(T_RSQBRACKET));
    else if (mode == CM_COMMA_OR_RBRACKET)   return ok_bool(accept(T_COMMA) || accept(T_RBRACKET));

    return failed_bool(NULL, "Unknown completion mode %d", mode);
}

static failable_expression parse_list_initializer(bool verbose, token *initial_token) {
    list *l = new_list(expression_class);

    // [] = empty list
    if (accept(T_RSQBRACKET))
        return ok_expression(new_list_data_expression(l, initial_token));

    // else parse expressions until we reach end square bracket.
    while (token_get_type(accepted()) != T_RSQBRACKET) {
        failable_expression expr = parse_expression(tokens_iterator, CM_COMMA_OR_RSQBRACKET, verbose);
        if (expr.failed) return failed_expression(&expr, NULL);
        list_add(l, expr.result);

        accept(T_RSQBRACKET); // allow superfluous commas: "a = [ 1, 2, ]"
    }

    return ok_expression(new_list_data_expression(l, initial_token));
}

static failable_expression parse_dict_initializer(bool verbose, token *initial_token) {
    dict *d = new_dict(expression_class);

    // {} = empty dict
    if (accept(T_RBRACKET))
        return ok_expression(new_dict_data_expression(d, initial_token));

    // else parse "key":expression until we reach end square bracket.
    while (token_get_type(accepted()) != T_RBRACKET) {
        failable_expression key_expr = parse_expression(tokens_iterator, CM_COLON, verbose);
        if (key_expr.failed) return failed_expression(&key_expr, NULL);
        if (expression_get_type(key_expr.result) != ET_IDENTIFIER)
            return failed_expression(NULL, "Dict keys should be identifiers, got %d", expression_get_type(key_expr.result));
        const char *key = expression_get_terminal_data(key_expr.result);

        failable_expression val_expr = parse_expression(tokens_iterator, CM_COMMA_OR_RBRACKET, verbose);
        if (val_expr.failed) return failed_expression(&val_expr, NULL);
        dict_set(d, key, val_expr.result);

        accept(T_RBRACKET); // allow superfluous commas: "a = { key1: 1, }"
    }

    return ok_expression(new_dict_data_expression(d, initial_token));
}

static failable_expression parse_func_declaration_expression(bool verbose, token *initial_token) {
    // past 'function', expected: "( [args] ) { [statements] }"
    if (!accept(T_LPAREN))
        return failed_expression(NULL, "Expected '(' after function");
    
    list *arg_names = new_list(str_class);
    while (!accept(T_RPAREN)) {
        if (!accept(T_IDENTIFIER))
            return failed_expression(NULL, "Expected identifier in function arg names");
        list_add(arg_names, (void *)token_get_data(accepted())); // we lose const here
        accept(T_COMMA);
    }

    failable_list statements_parsing = parse_statements(tokens_iterator, SP_BLOCK_MANDATORY);
    if (statements_parsing.failed) return failed_expression(&statements_parsing, "Failed parsing function body");

    return ok_expression(new_func_decl_expression(arg_names, statements_parsing.result, initial_token));
}

static failable parse_expression_on_want_operand(run_state *state, bool verbose) {

    // prefix operators come before the operand
    if (accept_positioned_operator(PREFIX)) {
        push_operator_pair(make_positioned_operator(accepted(), PREFIX), accepted());
        return ok();
    }

    // a parenthesis is a sub-expression here, func cals are handled after having operand.
    if (accept(T_LPAREN)) {
        failable_expression sub_expression = parse_expression(tokens_iterator, CM_RPAREN, verbose);
        if (sub_expression.failed) return failed(&sub_expression, "Subexpression failed");
        push_expression(sub_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "nums = [ 1, 2, 3, 5, 8, 13 ]"
    if (accept(T_LSQBRACKET)) {
        failable_expression list_expression = parse_list_initializer(verbose, accepted());
        if (list_expression.failed) return failed(&list_expression, "List initialization failed");
        push_expression(list_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "person = { name: "john", age: 30 };"
    if (accept(T_LBRACKET)) {
        failable_expression dict_expression = parse_dict_initializer(verbose, accepted());
        if (dict_expression.failed) return failed(&dict_expression, "Dict initialization failed");
        push_expression(dict_expression.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // e.g. "pie = function() { return 3.14; }"
    if (accept(T_FUNCTION_KEYWORD)) {
        failable_expression func_expression = parse_func_declaration_expression(verbose, accepted());
        if (func_expression.failed) return failed(&func_expression, "Parsing func declaration failed");
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
    return failed(NULL, "Unexpected token type %s at %s:%d:%d, was expecting operand or similar value construct", 
        token_type_str(token_get_type(peek())),
        token_get_file_name(peek()),
        token_get_file_line_no(peek()),
        token_get_file_col_no(peek())
    );
}

static failable_list parse_function_call_arguments_expressions(bool verbose) {
    list *args = new_list(expression_class);

    // if empty args, there will be nothing to parse
    if (accept(T_RPAREN))
        return ok_list(args);

    while (token_get_type(accepted()) != T_RPAREN) {
        failable_expression parse_arg = parse_expression(tokens_iterator, CM_COMMA_OR_RPAREN, verbose);
        if (parse_arg.failed)
            return failed_list(&parse_arg, NULL);
        list_add(args, parse_arg.result);
    }
    
    return ok_list(args);
}

static failable_expression parse_shorthand_if_pair(bool verbose) {
    failable_expression parsing = parse_expression(tokens_iterator, CM_COLON, verbose);
    if (parsing.failed) return failed_expression(&parsing, NULL);
    expression *e1 = parsing.result;
    token *colon_token = accepted();

    parsing = parse_expression(tokens_iterator, CM_END_OF_TEXT, verbose);
    if (parsing.failed) return failed_expression(&parsing, NULL);
    expression *e2 = parsing.result;

    return ok_expression(new_list_data_expression(list_of(expression_class, 2, e1, e2), colon_token));
}

static failable parse_expression_on_have_operand(run_state *state, completion_mode completion, bool verbose) {

    // if postfix, push for later, and remain in state
    if (accept_positioned_operator(POSTFIX)) {
        operator_type op = make_positioned_operator(accepted(), POSTFIX);
        create_expressions_for_higher_operators_than(op);
        push_operator_pair(op, accepted());
        return ok();
    }

    // special parsing of arguments after a function call parenthesis
    if (accept(T_LPAREN)) {
        token *initial_token = accepted();
        failable_list arg_expressions = parse_function_call_arguments_expressions(verbose);
        if (arg_expressions.failed)
            return failed(&arg_expressions, NULL);
        create_expressions_for_higher_operators_than(OP_FUNC_CALL);
        push_operator_pair(OP_FUNC_CALL, initial_token);
        push_expression(new_list_data_expression(arg_expressions.result, initial_token));
        *state = HAVE_OPERAND;
        return ok();
    }

    if (accept(T_QUESTION_MARK)) {
        token *initial_token = accepted();
        failable_expression if_parts = parse_shorthand_if_pair(verbose);
        if (if_parts.failed) return failed(&if_parts, NULL);
        create_expressions_for_higher_operators_than(OP_SHORT_IF);
        push_operator_pair(OP_SHORT_IF, initial_token);
        push_expression(if_parts.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // an array subscript, we must parse the ']'
    if (accept(T_LSQBRACKET)) {
        token *initial_token = accepted();
        failable_expression subscript = parse_expression(tokens_iterator, CM_RSQBRACKET, verbose);
        if (subscript.failed) return failed(&subscript, NULL);
        create_expressions_for_higher_operators_than(OP_ARRAY_SUBSCRIPT);
        push_operator_pair(OP_ARRAY_SUBSCRIPT, initial_token);
        push_expression(subscript.result);
        *state = HAVE_OPERAND;
        return ok();
    }

    // if infix, push it for resolving later, and go back to want-operand
    if (accept_positioned_operator(INFIX)) {
        operator_type op = make_positioned_operator(accepted(), INFIX);
        create_expressions_for_higher_operators_than(op);
        push_operator_pair(op, accepted());
        *state = WANT_OPERAND;
        return ok();
    }

    // detect if we finished (we may be a sub-expression)
    failable_bool completion_detection = detect_completion(completion);
    if (completion_detection.failed) return failed(&completion_detection, NULL);
    if (completion_detection.result) {
        create_expressions_for_higher_operators_than(OP_SENTINEL);
        *state = FINISHED;
        return ok();
    }
    
    // nothing else should be expected here
    return failed(NULL, "Unexpected token type %s at %s:%d:%d, was expecting operator_type or end", 
        token_type_str(token_get_type(peek())),
        token_get_file_name(peek()),
        token_get_file_line_no(peek()),
        token_get_file_col_no(peek())
    );
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
        fprintf(stderr, "    state=%s, accepted=%s, peek=%s\n",
            state_name,
            accepted() == NULL ? "NULL" : token_type_str(token_get_type(accepted())),
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
    push_operator_pair(OP_SENTINEL, NULL);

    while (state != FINISHED) {

        switch (state) {
            case WANT_OPERAND:
                state_handling = parse_expression_on_want_operand(&state, verbose);
                if (state_handling.failed)
                    return failed_expression(&state_handling, "Failed on want operand");
                break;
            case HAVE_OPERAND:
                state_handling = parse_expression_on_have_operand(&state, completion, verbose);
                if (state_handling.failed)
                    return failed_expression(&state_handling, "Failed on have operand");
                break;
        }

        if (verbose)
            print_debug_information("parse_expression() step", state);
    }

    pair *sentinel_pair = pop_top_operator_pair();
    if ((operator_type)pair_get_left(sentinel_pair) != OP_SENTINEL)
        return failed_expression(NULL, "Was expecting SENTINEL at the top of the queue");

    expression *result = pop_top_expression();

    if (verbose)
        print_debug_information("parse_expression() ended", state);
    
    return ok_expression(result);
}
