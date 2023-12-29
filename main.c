#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "value.h"
#include "dict.h"


/*
    The core of the functionality is the "evaluate()" function.
    It would give the ability to programs to evaluate strings,
    like complex business conditions etc.

    To do this we need a whole ecosystem for these:
    - values (strings, numbers, booleans, lists, dictionaries)
    - parser of expressions with precedence, into an abstract syntax tree
    - evaluation of the syntax tree, to find the result

    We would love to include function calls in the expression parser,
    something like "if(left(a, 1) == '0', 'number', 'letter')"

    evaluate("a + b * 2", {a=1, b=2})

    We should approach this from a tests / requirements perspective.
*/

value *evaluate(char *expression, dict *values);

// ---------------------------------------------------

static void verify_evaluation_null(char *expression) {
    dict *values = new_dict(10);
    value *returned = evaluate(expression, values);
    assert(value_is_null(returned));
}
static void verify_evaluation_b(char *expression, bool result) {
    dict *values = new_dict(10);
    value *returned = evaluate(expression, values);
    assert(value_as_bool(returned) == result);
}
static void verify_evaluation_bb(char *expression, bool a, bool result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_value(a));
    value *returned = evaluate(expression, values);
    assert(value_as_bool(returned) == result);
}
static void verify_evaluation_bbb(char *expression, bool a, bool b, bool result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_bool_value(a));
    dict_set(values, "b", new_bool_value(b));
    value *returned = evaluate(expression, values);
    assert(value_as_bool(returned) == result);
}
static void verify_evaluation_i(char *expression, int result) {
    dict *values = new_dict(10);
    value *returned = evaluate(expression, values);
    assert(value_as_int(returned) == result);
}
static void verify_evaluation_ii(char *expression, int a, int result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_value(a));
    value *returned = evaluate(expression, values);
    assert(value_as_int(returned) == result);
}
static void verify_evaluation_iii(char *expression, int a, int b, int result) {
    dict *values = new_dict(10);
    dict_set(values, "a", new_int_value(a));
    dict_set(values, "b", new_int_value(b));
    value *returned = evaluate(expression, values);
    assert(value_as_int(returned) == result);
}
static void verify_evaluation_ib(char *expression, int a, bool result) {
    dict *values = new_dict(10);
    value *returned = evaluate(expression, values);
    assert(value_as_bool(returned) == result);
}
static void test_evaluations() {
    /* Requirements:
        - constants (null, 0, 1, -2, 3.14, 'asdf', true / false)
        - addition, division, subtraction, multiplication
        - &&, ||
        - logical negation: "!false"
        - parentheses (e.g. "(1+2)*(3+4)" vs "1+(2*3)+4")
        - comparisons: ==, !=, <=, <, >=, >
        - predefined functions: if(cond, true, false), mod(10, 3), left(s, 3)
    */

    verify_evaluation_null(NULL);
    verify_evaluation_null("");
    verify_evaluation_null("null");
    verify_evaluation_null("1/0"); // error?

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

    verify_evaluation_ii("a", 4, 4);
    verify_evaluation_ii("a + 1", 4, 5);
    verify_evaluation_ii("a - 1", 4, 3);
    verify_evaluation_ii("a * 2", 4, 8);
    verify_evaluation_ii("a / 2", 4, 2);

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

    verify_evaluation_iii("if(a >  5, b, 1)",  5, 2, 8);
    verify_evaluation_iii("if(a >= 5, b, 1)", 5, 2, 2);
    verify_evaluation_iii("if(a == 5, b, 1)", 5, 3, 3);
    verify_evaluation_iii("if(a != 5, b, 1)", 5, 4, 1);

    // verify_evaluation_sb("left(a, 3) == 'har'", "harvard", true);
    // verify_evaluation_sb("right(a, 3) == 'ard'", "harvard", true);
    // verify_evaluation_sb("left(a, 10) == 'abc'", "abc", true);
    // verify_evaluation_sb("len(a) == 7", "harvard", true);
}

void main() {
    test_evaluations();
}


