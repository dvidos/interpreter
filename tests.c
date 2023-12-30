#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils/failable.h"
#include "utils/dict.h"
#include "utils/value.h"
#include "eval/eval.h"

static bool tests_failed;

#undef assert
#define assert(x)   if(!(x)) { \
        printf("Assertion failed '%s', at %s:%d\n", #x, __FILE__, __LINE__); \
        tests_failed = true; \
    }
#define assert_msg(x, msg)   if(!(x)) { \
        printf("Assertion failed '%s', %s, at %s:%d\n", #x, msg, __FILE__, __LINE__); \
        tests_failed = true; \
    }

#define fail(msg)      do { \
        printf("Failed: %s, at %s:%d\n", msg, __FILE__, __LINE__); \
        tests_failed = true; \
    } while(0)


static void verify_evaluation_null(char *expression) {
    dict *values = new_dict(10);
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_is_null(evaluation.result), expression);
}

static void verify_evaluation_b(char *expression, bool expected_result) {
    dict *values = new_dict(10);
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_bb(char *expression, bool a, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_value(a));
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_bbb(char *expression, bool a, bool b, bool expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_value(a));
    dict_set(values, "b", new_bool_value(b));
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_bool(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_i(char *expression, int expected_result) {
    dict *values = new_dict(10);
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_ii(char *expression, int a, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_value(a));
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_iii(char *expression, int a, int b, int expected_result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_value(a));
    dict_set(values, "b", new_int_value(b));
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_int(evaluation.result) == expected_result, expression);
}

static void verify_evaluation_ib(char *expression, int a, bool expected_result) {
    dict *values = new_dict(10);
    failable_value evaluation = evaluate(expression, values);
    if (evaluation.failed) { fail(evaluation.err_msg); return; }
    assert_msg(value_as_bool(evaluation.result) == expected_result, expression);
}

bool run_unit_tests() {
    tests_failed = false;

    /* Requirements:
        - constants (null, 0, 1, -2, 3.14, 'asdf', true / false)
        - addition, division, subtraction, multiplication
        - &&, ||
        - logical negation: "!false"
        - parentheses (e.g. "(1+2)*(3+4)" vs "1+(2*3)+4")
        - comparisons: ==, !=, <=, <, >=, >
        - predefined functions: if(cond, true, false), mod(10, 3), left(s, 3)
    */

    // verify_evaluation_null(NULL);
    // verify_evaluation_null("");
    // verify_evaluation_null("null");
    // verify_evaluation_null("1/0"); // error?

    // verify_evaluation_b("false", false);
    // verify_evaluation_b("true", true);
    // verify_evaluation_b("!false", true);
    // verify_evaluation_b("!true", false);
    // verify_evaluation_b("!(false)", true);
    // verify_evaluation_b("!(true)", false);
    // verify_evaluation_b("0", false);
    // verify_evaluation_b("1", true);
    // verify_evaluation_b("1234", true);

    // verify_evaluation_bb("a", true, true);
    // verify_evaluation_bb("a", false, false);
    // verify_evaluation_bb("!a", true, false);
    // verify_evaluation_bb("!a", false, true);

    // verify_evaluation_bbb("a && b", true, true, true);
    // verify_evaluation_bbb("a && b", true, false, false);
    // verify_evaluation_bbb("a && b", false, true, false);
    // verify_evaluation_bbb("a && b", false, false, false);
    // verify_evaluation_bbb("a || b", true, true, true);
    // verify_evaluation_bbb("a || b", true, false, true);
    // verify_evaluation_bbb("a || b", false, true, true);
    // verify_evaluation_bbb("a || b", false, false, false);

    // verify_evaluation_i("0", 0);
    // verify_evaluation_i("1", 1);
    // verify_evaluation_i("-1", -1);
    // verify_evaluation_i("1 + 2", 3);
    // verify_evaluation_i("2 * 3", 6);
    // verify_evaluation_i("5 - 2", 3);
    // verify_evaluation_i("8 / 2", 4);
    // verify_evaluation_i("1 + 2 * 3 + 4", 11);
    // verify_evaluation_i("1 + (2 * 3) + 4", 11);
    // verify_evaluation_i("(1 + 2) * (3 + 4)", 21);

    // verify_evaluation_ii("a", 4, 4);
    // verify_evaluation_ii("a + 1", 4, 5);
    // verify_evaluation_ii("a - 1", 4, 3);
    // verify_evaluation_ii("a * 2", 4, 8);
    // verify_evaluation_ii("a / 2", 4, 2);

    // verify_evaluation_ib("a > 5", 8, true);
    // verify_evaluation_ib("a > 5", 5, false);
    // verify_evaluation_ib("a >= 5", 5, true);
    // verify_evaluation_ib("a < 5", 3, true);
    // verify_evaluation_ib("a < 5", 5, false);
    // verify_evaluation_ib("a <= 5", 5, true);
    // verify_evaluation_ib("a == 5", 5, true);
    // verify_evaluation_ib("a == 5", 6, false);
    // verify_evaluation_ib("a != 5", 5, false);
    // verify_evaluation_ib("a != 5", 6, true);

    verify_evaluation_iii("if(a >  5, b, 1)",  5, 2, 8);
    verify_evaluation_iii("if(a >= 5, b, 1)", 5, 2, 2);
    verify_evaluation_iii("if(a == 5, b, 1)", 5, 3, 3);
    verify_evaluation_iii("if(a != 5, b, 1)", 5, 4, 1);

    return tests_failed;
}
