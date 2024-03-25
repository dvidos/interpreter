#include <string.h>
#include "../utils/testing.h"
#include "../utils/cstr.h"
#include "../utils/str.h"
#include "../utils/file.h"
#include "../utils/listing.h"
#include "../containers/_containers.h"
#include "interpreter.h"


enum acceptance_tests_token_type {
    TT_GIVEN,
    TT_CODE,
    TT_EXPECT_RESULT,
    TT_EXPECT_LOG,
    TT_EXPECT_VARIABLE,
    TT_EXPECT_EXCEPTION,
    TT_DATA,
    TT_UNKNOWN,
    TT_END
};

static struct parser_data {
    enum acceptance_tests_token_type type;
    char buffer[256];
    bool inside_code_block;
    bool inside_log_block;
} parser_data;

static struct acceptance_test {
    const char *filename;
    int line_no;
    str *title;
    dict *initial_vars; // items are str type, expressions
    str *code;
    str *expected_result; // str type, expressions
    dict *expected_vars_expressions;  // items are str type, expressions
    str *expected_log;
    bool expecting_exception;
} at;

static void initialize_acceptance_test() {
    at.filename = NULL;
    at.line_no = 0;
    at.title = new_str();
    at.initial_vars = new_dict(str_item_info);
    at.code = new_str();
    at.expected_result = new_str();
    at.expected_vars_expressions = new_dict(str_item_info);
    at.expected_log = new_str();
    at.expecting_exception = false;
}

static void teardown_acceptance_test() {
    str_free(at.title);
    dict_free(at.initial_vars);
    str_free(at.code);
    str_free(at.expected_result);
    dict_free(at.expected_vars_expressions);
    str_free(at.expected_log);
}

static bool reset_acceptance_test() {
    at.filename = NULL;
    at.line_no = 0;
    str_clear(at.title);
    dict_clear(at.initial_vars);
    str_clear(at.code);
    str_clear(at.expected_result);
    dict_clear(at.expected_vars_expressions);
    str_clear(at.expected_log);
    at.expecting_exception = false;
}

static variant *scripted_value_to_variant(str *s) {
    if (str_equals(s, "true"))
        return true_instance;
    if (str_equals(s, "false"))
        return false_instance;
    if (str_equals(s, "void"))
        return void_singleton;
    if (str_starts_with(s, "'") && str_ends_with(s, "'"))
        return new_str_variant(str_cstr(str_substr(s, 1, str_length(s) - 2)));
    if (str_starts_with(s, "\"") && str_ends_with(s, "\""))
        return new_str_variant(str_cstr(str_substr(s, 1, str_length(s) - 2)));

    // don't know what else!
    return new_int_variant(atoi(str_cstr(s)));
}

static void run_acceptance_test(bool with_debugger) {

    dict *values = new_dict(variant_item_info);
    for_dict(at.initial_vars, ivi, cstr, key) {
        variant *value = scripted_value_to_variant(dict_get(at.initial_vars, key));
        dict_set(values, key, value);
    }
    
    execution_outcome ex = interpret_and_execute(str_cstr(at.code), at.filename, values, false, with_debugger, with_debugger);
    if (ex.failed) {
        __testing_failed(ex.failure_message, str_cstr(at.title), at.filename, at.line_no);
        return;
    }
    
    if (ex.excepted) {
        if (at.expecting_exception) {
            __testing_passed();
        } else {
            __testing_failed(str_variant_as_str(variant_to_string(ex.exception_thrown)), str_cstr(at.title), at.filename, at.line_no);
        }
        return;
    }

    if (!str_empty(at.expected_result)) {
        variant *expected_value = scripted_value_to_variant(at.expected_result);
        if (!variants_are_equal(expected_value, ex.result)) {
            __testing_failed("Result comparison failed", str_cstr(at.title), at.filename, at.line_no);
            printf("  Expected: %s\n", str_cstr(at.expected_result));
            printf("  Actual  : %s\n", str_variant_as_str(variant_to_string(ex.result)));
        }
    }
    for_dict(at.expected_vars_expressions, evi, cstr, key) {
        str *expected_expr = dict_get(at.expected_vars_expressions, key);
        variant *expected = scripted_value_to_variant(expected_expr);
        variant *actual = dict_get(values, key);
        if (!variants_are_equal(expected, actual)) {
            __testing_failed("Expected result variable failed", str_cstr(at.title), at.filename, at.line_no);
            printf("  Key     : %s\n", key);
            printf("  Expected: %s\n", str_cstr(expected_expr));
            printf("  Actual  : %s\n", str_variant_as_str(variant_to_string(actual)));
        }
    }
    if (!str_empty(at.expected_log)) {
        if (strcmp(exec_context_get_log(), str_cstr(at.expected_log)) != 0) {
            __testing_failed("Log comparison failing", str_cstr(at.title), at.filename, at.line_no);
            printf("  Expected: %s\n", str_cstr(at.expected_log));
            printf("  Actual  : %s\n", exec_context_get_log());
        }
    }
    __testing_passed();
}

static bool parse_acceptance_test_line(const char *line_text, const char *filename, int line_no) {
    str *s = str_trim(new_str_of(line_text), " \t\r\n");
    if (str_starts_with(s, "//"))
        return true;

    if (str_equals(s, "```")) {
        if (parser_data.inside_code_block) {
            parser_data.inside_code_block = false;
        } else if (parser_data.inside_log_block) {
            parser_data.inside_log_block = false;
        } else {
            printf("Unexpected end-of-text (```), at %s:%d\n", filename, line_no);
            return false;
        }
        return true;
    }
    if (parser_data.inside_code_block) {
        str_add(at.code, s);
        str_addc(at.code, '\n');
        return true;
    } else if (parser_data.inside_log_block) {
        str_add(at.expected_log, s);
        str_addc(at.expected_log, '\n');
        return true;
    }

    // expect a first command: expect, result, code, etc.
    if (str_starts_with(s, "# ")) {
        str_add(at.title, str_substr(s, 2, 999));
    } else if (str_starts_with(s, "code ")) {
        if (str_ends_with(s, "```")) {
            parser_data.inside_code_block = true;
        } else {
            str_add(at.code, str_substr(s, 5, 1000));
        }
    } else if (str_starts_with(s, "expect result ")) {
        str_add(at.expected_result, str_substr(s, 14, 999));
    } else if (str_starts_with(s, "expect log ")) {
        if (str_ends_with(s, "```")) {
            parser_data.inside_log_block = true;
        } else {
            str_add(at.expected_log, str_substr(s, 11, 1000));
        }
    } else if (str_starts_with(s, "expect exception")) {
        at.expecting_exception = true;
    } else if (str_starts_with(s, "set ")) {
        // parse a comma-separated list of "a = 1"
        printf("Not yet supported (%s), at %s:%d\n", str_cstr(s), filename, line_no);
        return false;
    } else if (str_starts_with(s, "expect ")) {
        // parse a comma-separated list of "a == 1"
        printf("Not yet supported (%s), at %s:%d\n", str_cstr(s), filename, line_no);
        return false;
    } else {
        printf("Unexpected command \"%s\" at %s:%d\n", str_cstr(s), filename, line_no);
        return false;
    }

    return true;
}

bool run_acceptance_tests_from_text(const char *text, const char *filename, bool with_debugger) {
    printf("    %s\n", filename);
    initialize_acceptance_test();

    listing *l = new_listing(text);
    int count = listing_lines_count(l);

    reset_acceptance_test();
    for (int line_no = 1; line_no <= count; line_no++) {
        const char *line_text = listing_get_line(l, line_no);
        if (strlen(line_text) == 0)
            continue;

        // in the first non empty, non comment line
        if (at.filename == NULL) {
            at.filename = filename;
            at.line_no = line_no;
        }
        
        if (strncmp(line_text, "---", 3) == 0) {
            run_acceptance_test(with_debugger);
            reset_acceptance_test();
            continue;
        }

        if (!parse_acceptance_test_line(line_text, filename, line_no))
            return false;

    }
    if (!str_empty(at.code))
        run_acceptance_test(with_debugger);
    
    listing_free(l);
    teardown_acceptance_test();
    return true;
}

bool run_acceptance_tests_from_dir(const char *dirpath, const char *extension, bool with_debugger) {
    testing_initialize("acceptance");

    char **files = get_files(dirpath);
    if (files == NULL)
        return false;
    
    char *e;
    for (int i = 0; files[i] != NULL; i++) {
        e = find_extension(files[i]);
        if (e == NULL || strcmp(e, extension) != 0)
            continue;
        
        failable_const_char reading = file_read(files[i]);
        if (reading.failed)
            continue;
        
        if (!run_acceptance_tests_from_text(reading.result, files[i], with_debugger))
            return false;
        free((void *)reading.result);
    }
    free_files(files);

    return testing_outcome();
}
