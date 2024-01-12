#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/list.h"
#include "../utils/containers/stack.h"
#include "expression.h"
#include "token.h"
#include "tokenization.h"
#include "parser.h"
#include "eval.h"
#include "execution.h"


void initialize_evaluator() {
    initialize_char_tokens_trie();
    initialize_operator_tables();
}


failable_variant evaluate(const char *code, dict *arguments) {

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed)
        return failed("Tokenization failed: %s", tokenization.err_msg);
    
    failable_list parsing = parse_tokens_into_expressions(tokenization.result, false);
    if (parsing.failed)
        return failed("Parsing failed: %s", parsing.err_msg);

    variant *result = new_null_variant();
    for_list(parsing.result, results_iterator, expression, expr) {
        failable_variant execution = execute_expression(expr, arguments);
        if (execution.failed)
            return failed("Execution failed: %s", execution.err_msg);
        
        // if many expressions, the last result is kept and returned.
        result = execution.result;
    }

    return ok_variant(result);
}

// ------------------------------------------------------

static void verify_evaluation_null(char *expression) {
    dict *values = new_dict(10);
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_is_null(evaluation.result), expression);
}

static void verify_evaluation_b(char *expression, bool expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_bb(char *expression, bool a, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_variant(a));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_bbb(char *expression, bool a, bool b, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_variant(a));
    dict_set(values, "b", new_bool_variant(b));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_i(char *expression, int expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_ii(char *expression, int a, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_iii(char *expression, int a, int b, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    dict_set(values, "b", new_int_variant(b));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_ib(char *expression, int a, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_s(char *expression, char *expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(strcmp(variant_as_str(evaluation.result), expected_result) == 0, expression);
}

static void verify_evaluation_ss(char *expression, char *a, char *expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_str_variant(a));
    failable_variant evaluation = evaluate(expression, values);
    if (evaluation.failed) fail_test(evaluation.err_msg);
    assert_msg(strcmp(variant_as_str(evaluation.result), expected_result) == 0, expression);
}

bool evaluator_self_diagnostics() {
    bool passed = true;

    verify_evaluation_null(NULL);
    verify_evaluation_null("");
    // verify_evaluation_null("null");

    // verify_evaluation_failed("1/0");

    verify_evaluation_b("false", false);
    verify_evaluation_b("true", true);
    verify_evaluation_b("!false", true);
    verify_evaluation_b("!true", false);
    verify_evaluation_b("!(false)", true);
    verify_evaluation_b("!(true)", false);
    verify_evaluation_b("0", false);
    verify_evaluation_b("1", true);
    verify_evaluation_b("1234", true);

    verify_evaluation_bb("a", true, true);
    verify_evaluation_bb("a", false, false);
    verify_evaluation_bb("!a", true, false);
    verify_evaluation_bb("!a", false, true);

    verify_evaluation_bbb("a && b", true, true, true);
    verify_evaluation_bbb("a && b", true, false, false);
    verify_evaluation_bbb("a && b", false, true, false);
    verify_evaluation_bbb("a && b", false, false, false);
    verify_evaluation_bbb("a || b", true, true, true);
    verify_evaluation_bbb("a || b", true, false, true);
    verify_evaluation_bbb("a || b", false, true, true);
    verify_evaluation_bbb("a || b", false, false, false);

    verify_evaluation_i("0", 0);
    verify_evaluation_i("1", 1);
    verify_evaluation_i("-1", -1);
    verify_evaluation_i("1 + 2", 3);
    verify_evaluation_i("2 * 3", 6);
    verify_evaluation_i("5 - 2", 3);
    verify_evaluation_i("8 / 2", 4);
    verify_evaluation_i("1 + 2 * 3 + 4", 11);
    verify_evaluation_i("1 + (2 * 3) + 4", 11);
    verify_evaluation_i("(1 + 2) * (3 + 4)", 21);
    verify_evaluation_i("3;", 3);
    verify_evaluation_i("3; 5;", 5);
    verify_evaluation_i("3;6", 6);

    verify_evaluation_ii("a", 4, 4);
    verify_evaluation_ii("a + 1", 4, 5);
    verify_evaluation_ii("a - 1", 4, 3);
    verify_evaluation_ii("a * 2", 4, 8);
    verify_evaluation_ii("a / 2", 4, 2);
    verify_evaluation_ii("a / 3", 10, 3);
    verify_evaluation_ii("a++ + 3", 3, 6);
    verify_evaluation_ii("++a + 3", 3, 7);
    verify_evaluation_ii("a++; a", 5, 6);
    verify_evaluation_ii("a = a + 1; a", 5, 6);
    
    verify_evaluation_ib("a > 5", 8, true);
    verify_evaluation_ib("a > 5", 5, false);
    verify_evaluation_ib("a >= 5", 5, true);
    verify_evaluation_ib("a < 5", 3, true);
    verify_evaluation_ib("a < 5", 5, false);
    verify_evaluation_ib("a <= 5", 5, true);
    verify_evaluation_ib("a == 5", 5, true);
    verify_evaluation_ib("a == 5", 6, false);
    verify_evaluation_ib("a != 5", 5, false);
    verify_evaluation_ib("a != 5", 6, true);

    verify_evaluation_ii("a > 4 ? 5 : 6", 8, 5);
    verify_evaluation_ii("a > 4 ? 5 : 6", 2, 6);
    verify_evaluation_ii("a > 4 ? a + 1 : a + 2", 8, 9);

    verify_evaluation_s("''", "");
    verify_evaluation_s("'hello'", "hello");
    verify_evaluation_ss("a", "hello", "hello");
    verify_evaluation_ss("a + ' there'", "hello", "hello there");
    verify_evaluation_ss("a * 3", "-", "---");

    return passed;
}
