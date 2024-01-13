#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/list.h"
#include "../utils/containers/stack.h"
#include "parser/expression.h"
#include "parser/parser.h"
#include "lexer/token.h"
#include "lexer/tokenization.h"
#include "runtime/built_in_funcs.h"
#include "runtime/execution.h"
#include "interpreter.h"


static void verify_execution_failed(char *expression) {
    dict *values = new_dict(10);
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (!evaluation.failed)
        assertion_failed("Evaluation did not fail as expected");
    assertion_passed();
}

static void verify_execution_null(char *expression) {
    dict *values = new_dict(10);
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_is_null(evaluation.result), expression);
}

static void verify_execution_b(char *expression, bool expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_execution_bb(char *expression, bool a, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_variant(a));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_execution_bbb(char *expression, bool a, bool b, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_variant(a));
    dict_set(values, "b", new_bool_variant(b));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_execution_i(char *expression, int expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_execution_ii(char *expression, int a, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_execution_iii(char *expression, int a, int b, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    dict_set(values, "b", new_int_variant(b));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_int(evaluation.result) == expected_result, expression);
}

static void verify_execution_ib(char *expression, int a, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    assert_msg(variant_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_execution_s(char *expression, char *expected_result) {
    dict *values = new_dict(10);
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    bool answer_was_the_expected_one = strcmp(variant_as_str(evaluation.result), expected_result) == 0;
    assert_msg(answer_was_the_expected_one, expression);
}

static void verify_execution_ss(char *expression, char *a, char *expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_str_variant(a));
    failable_variant evaluation = interpret_and_execute(expression, values, false);
    if (evaluation.failed) { assertion_failed(evaluation.err_msg); return; }
    bool answer_was_the_expected_one = strcmp(variant_as_str(evaluation.result), expected_result) == 0;
    assert_msg(answer_was_the_expected_one, expression);
}

bool interpreter_self_diagnostics() {
    bool passed = true;

    verify_execution_null(NULL);
    verify_execution_null("");

    // can of worms, if "null" keyword should be supported!
    // verify_execution_null("null"); 

    verify_execution_b("false", false);
    verify_execution_b("true", true);
    verify_execution_b("!false", true);
    verify_execution_b("!true", false);
    verify_execution_b("!(false)", true);
    verify_execution_b("!(true)", false);
    verify_execution_b("0", false);
    verify_execution_b("1", true);
    verify_execution_b("1234", true);

    verify_execution_bb("a", true, true);
    verify_execution_bb("a", false, false);
    verify_execution_bb("!a", true, false);
    verify_execution_bb("!a", false, true);

    verify_execution_bbb("a && b", true, true, true);
    verify_execution_bbb("a && b", true, false, false);
    verify_execution_bbb("a && b", false, true, false);
    verify_execution_bbb("a && b", false, false, false);
    verify_execution_bbb("a || b", true, true, true);
    verify_execution_bbb("a || b", true, false, true);
    verify_execution_bbb("a || b", false, true, true);
    verify_execution_bbb("a || b", false, false, false);

    verify_execution_i("0", 0);
    verify_execution_i("1", 1);
    verify_execution_i("-1", -1);
    verify_execution_i("1 + 2", 3);
    verify_execution_i("2 * 3", 6);
    verify_execution_i("5 - 2", 3);
    verify_execution_i("8 / 2", 4);
    verify_execution_i("1 + 2 * 3 + 4", 11);
    verify_execution_i("1 + (2 * 3) + 4", 11);
    verify_execution_i("(1 + 2) * (3 + 4)", 21);
    verify_execution_i("3;", 3);
    verify_execution_i("3; 5;", 5);
    verify_execution_i("3;6", 6);
    verify_execution_failed("1/0");

    verify_execution_ii("a", 4, 4);
    verify_execution_ii("a + 1", 4, 5);
    verify_execution_ii("a - 1", 4, 3);
    verify_execution_ii("a * 2", 4, 8);
    verify_execution_ii("a / 2", 4, 2);
    verify_execution_ii("a / 3", 10, 3);
    verify_execution_ii("a++ + 3", 3, 6);
    verify_execution_ii("++a + 3", 3, 7);
    verify_execution_ii("a++; a", 5, 6);
    verify_execution_ii("a = a + 1; a", 5, 6);
    
    verify_execution_ib("a > 5", 8, true);
    verify_execution_ib("a > 5", 5, false);
    verify_execution_ib("a >= 5", 5, true);
    verify_execution_ib("a < 5", 3, true);
    verify_execution_ib("a < 5", 5, false);
    verify_execution_ib("a <= 5", 5, true);
    verify_execution_ib("a == 5", 5, true);
    verify_execution_ib("a == 5", 6, false);
    verify_execution_ib("a != 5", 5, false);
    verify_execution_ib("a != 5", 6, true);

    verify_execution_ii("a > 4 ? 5 : 6", 8, 5);
    verify_execution_ii("a > 4 ? 5 : 6", 2, 6);
    verify_execution_ii("a > 4 ? a + 1 : a + 2", 8, 9);

    verify_execution_s("''", "");
    verify_execution_s("'hello'", "hello");
    verify_execution_ss("a", "hello", "hello");
    verify_execution_ss("a + ' there'", "hello", "hello there");
    verify_execution_ss("a * 3", "-", "---");

    verify_execution_s("substr('hello there', 2, 3)", "llo");
    verify_execution_s("substr('hello there', 20, 3)", "");
    verify_execution_s("substr('hello there', 5, 0)", "");
    verify_execution_s("substr('hello there', 6, 200)", "there");
    verify_execution_s("substr('hello there', 200, 6)", "");
    verify_execution_s("substr('hello there', -5, 3)", "the");
    verify_execution_s("substr('hello there', 4, -2)", "o the");
    verify_execution_s("substr('hello there', -5, -2)", "the");

    verify_execution_failed("substr('hello there')");
    verify_execution_failed("some_function('hello there', 2, 3)");

    return passed;
}