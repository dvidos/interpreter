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
    EXP_LOG_CONTENTS,
} expected_outcome;

#define verify_execution(code, a_var, expected_outcome, out_var)  __verify_execution(__FILE__, __LINE__, code, a_var, expected_outcome, out_var)

static void __verify_execution(const char *file, int line, char *code, variant *var_a_value, expected_outcome expect_outcome, ...) {
    str_builder *sb = new_str_builder();
    dict *values = new_dict(variant_item_info);
    if (var_a_value != NULL)
        dict_set(values, "a", var_a_value);
    
    execution_outcome ex = interpret_and_execute(code, "verify_execution", values, false, false, false);

    if (expect_outcome == EXP_FAILURE) {
        if (ex.failed) assertion_passed();
        else __testing_failed("Evaluation did not fail as expected", code, file, line);
        return;
    }
    
    if (expect_outcome == EXP_EXCEPTION) {
        if (ex.exception_thrown) assertion_passed();
        else __testing_failed("Evaluation did not throw exception as expected", code, file, line);
        return;
    }

    // we expect some good result from now on       

    if (ex.failed) {
        str_builder_addf(sb, "Execution failed: %s", ex.failure_message);
        __testing_failed(str_builder_charptr(sb), code, file, line);
        return;
    }
    if (ex.exception_thrown) {
        str_builder_addf(sb, "Uncaught exception: %s", str_variant_as_str(variant_to_string(ex.exception)));
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

    } else if (expect_outcome == EXP_LOG_CONTENTS) {
        char *expected_log = va_arg(args, char *);
        const char *actual_log = exec_context_get_log();
        assert_strs_are_equal_fl(actual_log, expected_log, code, file, line);

    } else {
        assertion_failed("Unsupported expected result!", code);
    }
}

// static void verify_execution_log(char *code, char *expected_log) {
//     dict *values = new_dict(variant_item_info);
//     execution_outcome ex = interpret_and_execute(code, "test", values, false, false, false);
//     if (ex.failed)
//         assertion_failed(ex.failure_message, code);
//     else if (ex.exception_thrown)
//         assertion_failed("exception thrown", code);
//     else
//         assert_strs_are_equal(exec_context_get_log(), expected_log, code);
// }

void interpreter_self_diagnostics() {

    verify_execution("", NULL, EXP_VOID, NULL);

    // expressions that will cause exception
    verify_execution("1/0",                    NULL, EXP_EXCEPTION, NULL);
    verify_execution("some_var + 1",           NULL, EXP_EXCEPTION, NULL);
    verify_execution("some_function('hello')", NULL, EXP_EXCEPTION, NULL);

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

    // short hand if
    verify_execution("a > 4 ? 5 : 6",         new_int_variant(8), EXP_INTEGER, 5);
    verify_execution("a > 4 ? 5 : 6",         new_int_variant(2), EXP_INTEGER, 6);
    verify_execution("a > 4 ? a + 1 : a + 2", new_int_variant(8), EXP_INTEGER, 9);

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

    // reading and writing of container elements (list:num, dict:str)
    verify_execution("arr = [10,20,30]; return arr[2];",              NULL, EXP_INTEGER, 30);
    verify_execution("arr = []; arr[0] = 10; return arr[0];",         NULL, EXP_INTEGER, 10);
    verify_execution("arr = [10,20,30]; arr[1] = 22; return arr[1];", NULL, EXP_INTEGER, 22);
    verify_execution("man = {name:'Joe',age:40}; return man['age'];", NULL, EXP_INTEGER, 40);
    verify_execution("man = {}; max['age'] = 20; return man['age'];", NULL, EXP_INTEGER, 20);

    // set values, calculate on them
    verify_execution("i = 5; return i * i;", NULL, EXP_INTEGER, 25);


    verify_execution("log('abc', true, 123, -456);",
                     NULL, EXP_LOG_CONTENTS,
                     "abc true 123 -456\n");

    verify_execution("if (1 + 1 == 2) log('if-body'); else log('else-body');", 
                     NULL, EXP_LOG_CONTENTS,
                     "if-body\n");

    verify_execution("if (1 + 1 == 7) log('if-body'); else log('else-body');", 
                     NULL, EXP_LOG_CONTENTS,
                     "else-body\n");

    verify_execution("for (i = 0; i < 3; i++) log(i);",
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
