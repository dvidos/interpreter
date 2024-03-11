#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../containers/_module.h"
#include "../entities/_module.h"
#include "../lexer/_module.h"
#include "../parser/_module.h"
#include "../runtime/_module.h"
#include "interpreter.h"

typedef enum expected_outcome {
    EXP_FAILURE,
    EXP_EXCEPTION,
    EXP_VOID,
    EXP_BOOLEAN,
    EXP_INTEGER,
    EXP_STRING,
    EXP_LIST,
    EXP_LOG_CONTENTS,
} expected_outcome;

#define verify_execution(code, a_var, expected_outcome, out_var)  __verify_execution(__FILE__, __LINE__, code, a_var, expected_outcome, out_var)
static void __verify_execution(const char *file, int line, char *code, variant *var_a_value, expected_outcome expect_outcome, ...);


static void verify_basic_expressions() {

    // verify empty code returns a void result.
    verify_execution("", NULL, EXP_VOID, NULL);

    // primitive boolean literals
    verify_execution("false",    NULL, EXP_BOOLEAN, false);
    verify_execution("true",     NULL, EXP_BOOLEAN, true);
    verify_execution("!false",   NULL, EXP_BOOLEAN, true);
    verify_execution("!true",    NULL, EXP_BOOLEAN, false);
    verify_execution("!(false)", NULL, EXP_BOOLEAN, true);
    verify_execution("!(true)",  NULL, EXP_BOOLEAN, false);

    // boolean variables
    verify_execution("a",  new_bool_variant(true),  EXP_BOOLEAN, true);
    verify_execution("a",  new_bool_variant(false), EXP_BOOLEAN, false);
    verify_execution("!a", new_bool_variant(true),  EXP_BOOLEAN, false);
    verify_execution("!a", new_bool_variant(false), EXP_BOOLEAN, true);

    // primitive logic operations
    verify_execution("true  && true ", NULL, EXP_BOOLEAN, true);
    verify_execution("true  && false", NULL, EXP_BOOLEAN, false);
    verify_execution("false && true ", NULL, EXP_BOOLEAN, false);
    verify_execution("false && false", NULL, EXP_BOOLEAN, false);
    verify_execution("true  || true ", NULL, EXP_BOOLEAN, true);
    verify_execution("true  || false", NULL, EXP_BOOLEAN, true);
    verify_execution("false || true ", NULL, EXP_BOOLEAN, true);
    verify_execution("false || false", NULL, EXP_BOOLEAN, false);
}

static void verify_branching_logic() {

    // short hand if
    verify_execution("a > 4 ? 5 : 6",         new_int_variant(8), EXP_INTEGER, 5);
    verify_execution("a > 4 ? 5 : 6",         new_int_variant(2), EXP_INTEGER, 6);
    verify_execution("a > 4 ? a + 1 : a + 2", new_int_variant(8), EXP_INTEGER, 9);

    verify_execution("log('abc', true, 123, -456);",
                     NULL, EXP_LOG_CONTENTS,
                     "abc true 123 -456\n");

    verify_execution("if (1 + 1 == 2)"
                     "    log('if-body');"
                     "else"
                     "    log('else-body');", 
                     NULL, EXP_LOG_CONTENTS,
                     "if-body\n");

    verify_execution("if (1 + 1 == 7)"
                     "    log('if-body');"
                     "else"
                     "    log('else-body');", 
                     NULL, EXP_LOG_CONTENTS,
                     "else-body\n");

    verify_execution("for (i = 0; i < 3; i++)"
                     "   log(i);",
                     NULL, EXP_LOG_CONTENTS,
                     "0\n1\n2\n");
    
    verify_execution("for (i = 0; i < 10; i++) {"
                     "   if (i < 3) continue;"
                     "   log(i);"
                     "   if (i >= 6) break;"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "3\n4\n5\n6\n");
    
    verify_execution("i = 5; while (i > 0) {"
                     "   log(i--);"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "5\n4\n3\n2\n1\n");
    
    verify_execution("i = 0; while (i++ < 10) {"
                     "   log(i);"
                     "   if (i >= 5)"
                     "       return i;"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "1\n2\n3\n4\n5\n");
}

static void verify_int_expressions() {
    // primitive numbers
    verify_execution("0",                 NULL, EXP_INTEGER,  0);
    verify_execution("1",                 NULL, EXP_INTEGER,  1);
    verify_execution("-1",                NULL, EXP_INTEGER, -1);
    verify_execution("1 + 2",             NULL, EXP_INTEGER,  3);
    verify_execution("2 * 3",             NULL, EXP_INTEGER,  6);
    verify_execution("5 - 2",             NULL, EXP_INTEGER,  3);
    verify_execution("8 / 2",             NULL, EXP_INTEGER,  4);
    verify_execution("1 + 2 * 3 + 4",     NULL, EXP_INTEGER, 11);
    verify_execution("1 + (2 * 3) + 4",   NULL, EXP_INTEGER, 11);
    verify_execution("(1 + 2) * (3 + 4)", NULL, EXP_INTEGER, 21);
    verify_execution("3",                 NULL, EXP_INTEGER,  3);
    verify_execution("3;",                NULL, EXP_INTEGER,  3);
    verify_execution("3 ;",               NULL, EXP_INTEGER,  3);

    // numeric manipulation, by variable
    verify_execution("a",         new_int_variant(4),  EXP_INTEGER, 4);
    verify_execution("a + 1",     new_int_variant(4),  EXP_INTEGER, 5);
    verify_execution("a - 1",     new_int_variant(4),  EXP_INTEGER, 3);
    verify_execution("a * 2",     new_int_variant(4),  EXP_INTEGER, 8);
    verify_execution("a / 2",     new_int_variant(4),  EXP_INTEGER, 2);
    verify_execution("a / 3",     new_int_variant(10), EXP_INTEGER, 3);
    verify_execution("a++ + 3",   new_int_variant(3),  EXP_INTEGER, 6);
    verify_execution("++a + 3",   new_int_variant(3),  EXP_INTEGER, 7);
    verify_execution("a = a + 1", new_int_variant(5),  EXP_INTEGER, 6);
    
    // comparisons
    verify_execution("a > 5",  new_int_variant(8), EXP_BOOLEAN, true);
    verify_execution("a > 5",  new_int_variant(5), EXP_BOOLEAN, false);
    verify_execution("a >= 5", new_int_variant(5), EXP_BOOLEAN, true);
    verify_execution("a < 5",  new_int_variant(3), EXP_BOOLEAN, true);
    verify_execution("a < 5",  new_int_variant(5), EXP_BOOLEAN, false);
    verify_execution("a <= 5", new_int_variant(5), EXP_BOOLEAN, true);
    verify_execution("a == 5", new_int_variant(5), EXP_BOOLEAN, true);
    verify_execution("a == 5", new_int_variant(6), EXP_BOOLEAN, false);
    verify_execution("a != 5", new_int_variant(5), EXP_BOOLEAN, false);
    verify_execution("a != 5", new_int_variant(6), EXP_BOOLEAN, true);

    // set values, calculate on them
    verify_execution("i = 5; return i * i;", NULL, EXP_INTEGER, 25);
}

static void verify_string_expressions() {

    // the substr() method
    verify_execution("''",                            NULL, EXP_STRING, "");
    verify_execution("'hello'",                       NULL, EXP_STRING, "hello");
    verify_execution("substr('hello there', 2, 3)",   NULL, EXP_STRING, "llo");
    verify_execution("substr('hello there', 20, 3)",  NULL, EXP_STRING, "");
    verify_execution("substr('hello there', 5, 0)",   NULL, EXP_STRING, "");
    verify_execution("substr('hello there', 6, 200)", NULL, EXP_STRING, "there");
    verify_execution("substr('hello there', 200, 6)", NULL, EXP_STRING, "");
    verify_execution("substr('hello there', -5, 3)",  NULL, EXP_STRING, "the");
    verify_execution("substr('hello there', 4, -2)",  NULL, EXP_STRING, "o the");
    verify_execution("substr('hello there', -5, -2)", NULL, EXP_STRING, "the");

    // string manipulations
    verify_execution("a",            new_str_variant("hello"), EXP_STRING, "hello");
    verify_execution("a + ' there'", new_str_variant("hello"), EXP_STRING, "hello there");
    verify_execution("a * 3",        new_str_variant("-"),     EXP_STRING, "---");
    
}

static void verify_list_expressions() {
    verify_execution("arr = []; return arr.empty();",      NULL, EXP_BOOLEAN, true);
    verify_execution("arr = [1]; return arr.empty();",     NULL, EXP_BOOLEAN, false);
    verify_execution("arr = []; return arr.length();",     NULL, EXP_INTEGER, 0);
    verify_execution("arr = [1]; return arr.length();",    NULL, EXP_INTEGER, 1);
    verify_execution("arr = [1, 2]; return arr.length();", NULL, EXP_INTEGER, 2);

    verify_execution("arr = [10,20,30]; return arr[2];",              NULL, EXP_INTEGER, 30);
    verify_execution("arr = []; arr[0] = 10; return arr[0];",         NULL, EXP_INTEGER, 10);
    verify_execution("arr = [10,20,30]; arr[1] = 22; return arr[1];", NULL, EXP_INTEGER, 22);
    verify_execution("a = []; a[200] = 1;",                           NULL, EXP_EXCEPTION, NULL);

    verify_execution("arr = [1, 2, 3, 4];"
                     "f = function(item, idx, arr){return item % 2 == 0;};"
                     "return arr.filter(f);",
                     NULL, EXP_LIST,
                     list_of(variant_item_info, 2, new_int_variant(2), new_int_variant(4)));

    verify_execution("arr = [1, 2, 3];"
                     "f = function(item, idx, arr){return item * 2;};"
                     "return arr.map(f);",
                     NULL, EXP_LIST,
                     list_of(variant_item_info, 3, new_int_variant(2), new_int_variant(4), new_int_variant(6)));

    verify_execution("arr = [1, 2, 3];"
                     "f = function(acc, item, idx, arr){return acc + item;};"
                     "return arr.reduce(0, f);",
                     NULL, EXP_INTEGER, 6);
}

static void verify_dict_expressions() {

    verify_execution("man = {name:'Joe',age:40}; return man['age'];", NULL, EXP_INTEGER, 40);
    verify_execution("man = {}; man['age'] = 20; return man['age'];", NULL, EXP_INTEGER, 20);
    
}

static void verify_exception_handling() {

    // expressions that will cause exception
    verify_execution("1/0",                    NULL, EXP_EXCEPTION, NULL);
    verify_execution("some_var + 1",           NULL, EXP_EXCEPTION, NULL);
    verify_execution("some_function('hello')", NULL, EXP_EXCEPTION, NULL);

    verify_execution("try {"
                     "    log('in try block');"
                     "} catch (e) {"
                     "    log('in catch block');"
                     "} finally {"
                     "    log('in finally block');"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "in try block\n"
                     "in finally block\n");
    
    verify_execution("try {"
                     "    log('before throwing');"
                     "    throw 'hello';"
                     "    log('after throwing');"
                     "} catch (e) {"
                     "    log('caught exception');"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "before throwing\n"
                     "caught exception\n");
    
    verify_execution("try {"
                     "    try {"
                     "        log('before throwing');"
                     "        throw 'hello';"
                     "        log('after throwing');"
                     "    } finally {"
                     "        log('in finally block');"
                     "    }"
                     "} catch (e) {"
                     "   log('exception caught here');"
                     "}",
                     NULL, EXP_LOG_CONTENTS,
                     "before throwing\n"
                     "in finally block\n"
                     "exception caught here\n");

}

static void verify_classes_handling() {
    
    // access should be private by default
    verify_execution("class T { a1 = 313; }"
                     "c = new(T);"
                     "return c.a1;",
                     NULL, EXP_EXCEPTION, NULL);

    // but could be made public if one wants
    verify_execution("class T { public a1 = 313; }"
                     "c = new(T);"
                     "return c.a1;",
                     NULL, EXP_EXCEPTION, NULL);

    // simple class
    verify_execution("class T {"
                     "    a1 = 313;"
                     "    function giveme() { return this.a1; }"
                     "}"
                     "c = new(T);"
                     "return c.giveme();",
                     NULL, EXP_INTEGER, 313);

    // uninitialized attribute
    verify_execution("class T {"
                     "    a1;"
                     "    function giveme() { return this.a1; }"
                     "}"
                     "c = new(T);"
                     "return c.giveme();",
                     NULL, EXP_VOID, NULL);

    // constructor parameter
    verify_execution("class T {"
                     "    a1 = 313;"
                     "    function construct(num) { this.a1 = num; }"
                     "    function giveme() { return this.a1; }"
                     "}"
                     "c = new(T, 626);"
                     "return c.giveme();",
                     NULL, EXP_INTEGER, 626);
}

static void verify_function_creation_and_calling() {
    // call named function
    verify_execution("function increaser(base) { return base + 1; }"
                     "return increaser(1);",
                     NULL, EXP_INTEGER, 2);
                    
    // call anonymous method
    verify_execution("increaser = function (base) { return base + 1; };"
                     "return increaser(1);",
                     NULL, EXP_INTEGER, 2);
                    
    // TODO: implement support for capturing env variables in closures.
    // // closure captures env values
    // verify_execution("function make_reminder(number) {"
    //                  "    return function() { return number; };"
    //                  "}"
    //                  "r = make_reminder(2);"
    //                  "return r();",
    //                  NULL, EXP_INTEGER, 2);

    // // closure modifies captured values
    // verify_execution("function make_sequencer(base) {"
    //                  "    return function() { return ++base; };"
    //                  "}"
    //                  "s = make_sequencer(1);"
    //                  "return s();",
    //                  NULL, EXP_INTEGER, 2);

    // // call method on object
    // verify_execution("class sequencer { base = 1; function next() { return ++(this.base); }; }"
    //                  "s = new(sequencer);"
    //                  "return s.next();", 
    //                  NULL, EXP_INTEGER,
    //                  2);
                    
    // // promote method to callable and call it.
    // verify_execution("class sequencer { base = 1; function next() { return ++(this.base); }; }"
    //                  "s = new(sequencer);"
    //                  "f = s.next;" 
    //                  "return f();",
    //                  NULL, EXP_INTEGER,
    //                  2);
}

void interpreter_self_diagnostics() {

    verify_basic_expressions();
    verify_branching_logic();
    verify_int_expressions();
    verify_string_expressions();
    verify_list_expressions();
    verify_dict_expressions();
    verify_exception_handling();
    verify_classes_handling();
    verify_function_creation_and_calling();
}

static void __verify_execution(const char *file, int line, char *code, variant *var_a_value, expected_outcome expect_outcome, ...) {
    str_builder *sb = new_str_builder();
    dict *values = new_dict(variant_item_info);
    if (var_a_value != NULL)
        dict_set(values, "a", var_a_value);
    
    execution_outcome ex = interpret_and_execute(code, "test_code", values, false, false, false);

    if (expect_outcome == EXP_FAILURE) {
        if (ex.failed) assertion_passed();
        else __testing_failed("Evaluation did not fail as expected", code, file, line);
        return;
    }
    
    if (expect_outcome == EXP_EXCEPTION) {
        if (ex.excepted) assertion_passed();
        else __testing_failed("Evaluation did not throw exception as expected", code, file, line);
        return;
    }

    // we expect some good result from now on       

    if (ex.failed) {
        str_builder_addf(sb, "Execution failed: %s", ex.failure_message);
        __testing_failed(str_builder_charptr(sb), code, file, line);
        return;
    }
    if (ex.excepted) {
        str_builder_addf(sb, "Uncaught exception: %s", str_variant_as_str(variant_to_string(ex.exception_thrown)));
        __testing_failed(str_builder_charptr(sb), code, file, line);
        return;
    }

    variant *actual_result = ex.result;
    va_list args;
    va_start(args, expect_outcome);

    if (expect_outcome == EXP_VOID) {
        assert_variant_is_of_type_fl(actual_result, void_type, code, file, line);

    } else if (expect_outcome == EXP_BOOLEAN) {
        // warning: ‘_Bool’ is promoted to ‘int’ when passed through ‘...’
        bool expected_result = (bool)va_arg(args, int);
        assert_variant_has_bool_value_fl(actual_result, expected_result, code, file, line);

    } else if (expect_outcome == EXP_INTEGER) {
        int expected_result = va_arg(args, int);
        assert_variant_has_int_value_fl(actual_result, expected_result, code, file, line);

    } else if (expect_outcome == EXP_STRING) {
        char *expected_result = va_arg(args, char *);
        assert_variant_has_str_value_fl(actual_result, expected_result, code, file, line);

    } else if (expect_outcome == EXP_LIST) {
        if (!variant_instance_of(actual_result, list_type)) {
            str_builder_addf(sb, "Was expecting a list result, got '%s'", actual_result->_type->name);
            __testing_failed(str_builder_charptr(sb), code, file, line);
        } else {
            list *expected_contents = va_arg(args, list *);
            assert_lists_are_equal_fl(list_variant_as_list(actual_result), expected_contents, code, file, line);
        }

    } else if (expect_outcome == EXP_LOG_CONTENTS) {
        char *expected_log = va_arg(args, char *);
        const char *actual_log = exec_context_get_log();
        assert_strs_are_equal_fl(actual_log, expected_log, code, file, line);

    } else {
        assertion_failed("Unsupported expected result!", code);
    }
}
