#ifndef _TESTING_H
#define _TESTING_H

#include <stdbool.h>

#ifdef assert
#undef assert
#endif


void testing_initialize();
void __testing_passed();
void __testing_failed(const char *test_descr, const char *extra, const char *file, int line);
void __testing_assert(bool passed, const char *test_descr, const char *extra, const char *file, int line);
bool testing_outcome();




#define assert(x)                            __testing_assert(x, #x, NULL, __FILE__, __LINE__)
#define assert_msg(x, extra)                 __testing_assert(x, #x, extra, __FILE__, __LINE__)
#define assertion_passed()                   __testing_passed()
#define assertion_failed(test_descr, extra)  __testing_failed(test_descr, extra, __FILE__, __LINE__)

#define assert_true(x, extra)                __testing_assert((x), #x " is not true", extra, __FILE__, __LINE__)
#define assert_false(x, extra)               __testing_assert(!(x), #x " is not false", extra, __FILE__, __LINE__)
#define assert_null(x, extra)                __testing_assert((x) == NULL, #x " is not null", extra, __FILE__, __LINE__)


#define assert_bools_are_equal_fl(actual, expected, extra, file, line)  \
    if (actual == expected) {  \
        __testing_passed();  \
    } else {  \
        __testing_failed("Boolean values are not equal", extra, file, line);  \
        printf("    Expected: %s\n", expected ? "true" : "false");  \
        printf("    Actual  : %s\n", actual ? "true" : "false");  \
    }

#define assert_ints_are_equal_fl(actual, expected, extra, file, line)  \
    if (actual == expected) {  \
        __testing_passed();  \
    } else {  \
        __testing_failed("Integer values are not equal", extra, file, line);  \
        printf("    Expected: %d\n", expected);  \
        printf("    Actual  : %d\n", actual);  \
    }

#define assert_strs_are_equal_fl(actual, expected, extra, file, line)  \
    if (strcmp(actual, expected) == 0) {  \
        __testing_passed();  \
    } else {  \
        __testing_failed("String values are not equal", extra, file, line);  \
        printf("    Expected: %s\n", expected);  \
        printf("    Actual  : %s\n", actual);  \
    }


#define assert_str_builder_equals(sb, expected_string, extra)   \
    if (strcmp(str_builder_charptr(sb), expected_string) == 0) { \
        __testing_passed(); \
    } else { \
        __testing_failed("str_builder content not expected", extra, __FILE__, __LINE__);  \
        printf("    Expected: %s\n", expected_string);  \
        printf("    Actual  : %s\n", str_builder_charptr(sb));  \
    }


#define assert_lists_are_equal_fl(actual, expected, extra, file, line)  \
    if (lists_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Lists are not equal", extra, file, line);  \
        str_builder *sb_e = new_str_builder();  \
        str_builder *sb_a = new_str_builder();  \
        list_describe(expected, ", ", sb_e);  \
        list_describe(actual, ", ", sb_a);  \
        printf("    Expected: %s\n", str_builder_charptr(sb_e));  \
        printf("    Actual  : %s\n", str_builder_charptr(sb_a));  \
    }

#define assert_dicts_are_equal(actual, expected, extra) \
    if (dicts_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Dictionaries are not equal", extra, __FILE__, __LINE__);  \
        str_builder *sb_e = new_str_builder();  \
        str_builder *sb_a = new_str_builder();  \
        dict_describe(expected, sb_e);  \
        dict_describe(actual, sb_a);  \
        printf("    Expected: %s\n", str_builder_charptr(sb_e));  \
        printf("    Actual  : %s\n", str_builder_charptr(sb_a));  \
    }

#define assert_variant_is_of_type_fl(actual_variant, expected_type, extra, file, line) \
    if (variant_instance_of(actual_variant, expected_type)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Variant is not of the expected type", extra, file, line);  \
        printf("    Expected: %s\n", expected_type->name);  \
        printf("    Actual  : %s\n", actual_variant->_type->name);  \
    }

#define assert_variants_are_equal_fl(actual, expected, extra, file, line) \
    if (variants_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Variants are not equal", extra, file,);  \
        printf("    Expected: %s\n", str_variant_as_str(variant_to_string(expected)));  \
        printf("    Actual  : %s\n", str_variant_as_str(variant_to_string(actual)));  \
    }

#define assert_variants_are_equal(actual, expected, extra) \
    if (variants_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Variants are not equal", extra, __FILE__, __LINE__);  \
        printf("    Expected: %s\n", str_variant_as_str(variant_to_string(expected)));  \
        printf("    Actual  : %s\n", str_variant_as_str(variant_to_string(actual)));  \
    }

#define assert_tokens_are_equal(actual, expected, extra) \
    if (tokens_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Tokens are not equal", extra, __FILE__, __LINE__);  \
        str_builder *sb_e = new_str_builder();  \
        str_builder *sb_a = new_str_builder();  \
        token_describe(expected, sb_e);  \
        token_describe(actual, sb_a);  \
        printf("    Expected: %s\n", str_builder_charptr(sb_e));  \
        printf("    Actual  : %s\n", str_builder_charptr(sb_a));  \
    }

#define assert_expressions_are_equal(actual, expected, extra) \
    if (expressions_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Expressions are not equal", extra, __FILE__, __LINE__);  \
        str_builder *sb_e = new_str_builder();  \
        str_builder *sb_a = new_str_builder();  \
        expression_describe(expected, sb_e);  \
        expression_describe(actual, sb_a);  \
        printf("    Expected: %s\n", str_builder_charptr(sb_e));  \
        printf("    Actual  : %s\n", str_builder_charptr(sb_a));  \
    }

#define assert_statements_are_equal(actual, expected, extra) \
    if (statements_are_equal(actual, expected)) {  \
        __testing_passed(); \
    } else { \
        __testing_failed("Statements are not equal", extra, __FILE__, __LINE__);  \
        str_builder *sb_e = new_str_builder();  \
        str_builder *sb_a = new_str_builder();  \
        statement_describe(expected, sb_e);  \
        statement_describe(actual, sb_a);  \
        printf("    Expected: %s\n", str_builder_charptr(sb_e));  \
        printf("    Actual  : %s\n", str_builder_charptr(sb_a));  \
    }


#define assert_variant_has_bool_value_fl(var, expected_value, extra, file, line) \
    if (!variant_instance_of(var, bool_type)) { \
        __testing_failed("Variant is not a bool", extra, file, line); \
    } else if (bool_variant_as_bool(var) != expected_value) { \
        __testing_failed("Variant does not have expected value", extra, file, line); \
        printf("    Expected: %s\n", expected_value ? "true" : "false");  \
        printf("    Actual  : %s\n", bool_variant_as_bool(var) ? "true" : "false");  \
    } else { \
        __testing_passed(); \
    }

#define assert_variant_has_int_value_fl(var, expected_value, extra, file, line) \
    if (!variant_instance_of(var, int_type)) { \
        __testing_failed("Variant is not an int", extra, file, line); \
    } else if (int_variant_as_int(var) != expected_value) { \
        __testing_failed("Variant does not have expected value", extra, file, line); \
        printf("    Expected: %d\n", expected_value);  \
        printf("    Actual  : %d\n", int_variant_as_int(var));  \
    } else { \
        __testing_passed(); \
    }

#define assert_variant_has_str_value_fl(var, expected_value, extra, file, line) \
    if (!variant_instance_of(var, str_type)) {  \
        __testing_failed("Variant is not a string", extra, file, line);  \
    } else if (strcmp(str_variant_as_str(var), expected_value) != 0) {  \
        __testing_failed("Variant does not have expected value", extra, file, line);  \
        printf("    Expected: %s\n", expected_value);  \
        printf("    Actual  : %s\n", str_variant_as_str(var));  \
    } else { \
        __testing_passed(); \
    }




#endif
