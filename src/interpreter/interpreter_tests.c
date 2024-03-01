#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../entities/_module.h"
#include "../lexer/_module.h"
#include "../parser/_module.h"
#include "../runtime/_module.h"
#include "interpreter.h"


static void verify_execution_failed(char *code) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (!evaluation.failed)
        assertion_failed("Evaluation did not fail as expected", code);
    else
        assertion_passed();
}

static void verify_execution_exceptioned(char *code) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_passed();
    else
        assertion_failed("Evaluation did not throw exception as expected", code);
}

static void verify_execution_null(char *code) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else if (!variant_is_null(evaluation.result))
        assertion_failed("Result is not null", code);
    else
        assertion_passed();
}

static void verify_execution_b(char *code, bool expected_result) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_bool_value(evaluation.result, expected_result, code);
}

static void verify_execution_bb(char *code, bool a, bool expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_bool_variant(a));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_bool_value(evaluation.result, expected_result, code);
}

static void verify_execution_bbb(char *code, bool a, bool b, bool expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_bool_variant(a));
    dict_set(values, "b", new_bool_variant(b));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_bool_value(evaluation.result, expected_result, code);
}

static void verify_execution_i(char *code, int expected_result) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_int_value(evaluation.result, expected_result, code);
}

static void verify_execution_ii(char *code, int a, int expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_int_value(evaluation.result, expected_result, code);
}

static void verify_execution_iii(char *code, int a, int b, int expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_int_variant(a));
    dict_set(values, "b", new_int_variant(b));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_int_value(evaluation.result, expected_result, code);
}

static void verify_execution_ib(char *code, int a, bool expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_int_variant(a));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_bool_value(evaluation.result, expected_result, code);
}

static void verify_execution_s(char *code, char *expected_result) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_str_value(evaluation.result, expected_result, code);
}

static void verify_execution_ss(char *code, char *a, char *expected_result) {
    dict *values = new_dict(variant_class);
    dict_set(values, "a", new_str_variant(a));
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_variant_has_str_value(evaluation.result, expected_result, code);
}

static void verify_execution_log(char *code, char *expected_log) {
    dict *values = new_dict(variant_class);
    failable_variant evaluation = interpret_and_execute(code, "test", values, false, false, false);
    if (evaluation.failed)
        assertion_failed(evaluation.err_msg, code);
    else if (variant_is_exception(evaluation.result))
        assertion_failed("exception thrown", code);
    else
        assert_strs_are_equal(exec_context_get_log(), expected_log, code);
}

void interpreter_self_diagnostics() {

    // can of worms, if "null" keyword should be supported!
    // verify_execution_null(NULL);
    // verify_execution_null("");
    // verify_execution_null("null"); 

    verify_execution_b("false", false);
    verify_execution_b("true", true);
    verify_execution_b("!false", true);
    verify_execution_b("!true", false);
    verify_execution_b("!(false)", true);
    verify_execution_b("!(true)", false);

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
    verify_execution_i("3", 3);
    verify_execution_i("3;", 3);
    verify_execution_i("3 ;", 3);
    verify_execution_failed("1/0");

    verify_execution_ii("a", 4, 4);
    verify_execution_ii("a + 1", 4, 5);
    verify_execution_ii("a - 1", 4, 3);
    verify_execution_ii("a * 2", 4, 8);
    verify_execution_ii("a / 2", 4, 2);
    verify_execution_ii("a / 3", 10, 3);
    verify_execution_ii("a++ + 3", 3, 6);
    verify_execution_ii("++a + 3", 3, 7);
    verify_execution_ii("a = a + 1", 5, 6);
    
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

    verify_execution_failed("some_function('hello there', 2, 3)");

    verify_execution_log("log('abc', true, 123, -456);",
                         "abc true 123 -456\n");

    verify_execution_log("if (1 + 1 == 2) log('if-body'); else log('else-body');", 
                         "if-body\n");

    verify_execution_log("if (1 + 1 == 7) log('if-body'); else log('else-body');", 
                         "else-body\n");

    verify_execution_log("for (i = 0; i < 3; i++) log(i);",
                         "0\n1\n2\n");
    
    verify_execution_log("for (i = 0; i < 10; i++) {"
                         "   if (i < 3) continue;"
                         "   log(i);"
                         "   if (i >= 6) break;"
                         "}",
                         "3\n4\n5\n6\n");
    
    verify_execution_log("i = 5; while (i > 0) {"
                         "   log(i--);"
                         "}",
                         "5\n4\n3\n2\n1\n");
    
    verify_execution_log("i = 0; while (i++ < 10) {"
                         "   log(i);"
                         "   if (i >= 5)"
                         "       return i;"
                         "}",
                         "1\n2\n3\n4\n5\n");
    
    verify_execution_i("i = 5; return i * i;", 25);

    setenv("ENV_VAR_A", "some-value", true);
    verify_execution_b("getenv('ENV_VAR_A') == 'some-value'", true); // bare expr format
    verify_execution_b("return (getenv('ENV_VAR_A') == 'some-value');", true); // statement format
}
