#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "built_in_funcs_tests.h"
#include "../interpreter/interpreter.h"
#include "../utils/data_types/_module.h"
#include "../utils/testing.h"


static bool validate_use_case(const char *code, variant *expected, bool verbose) {
    dict *values = new_dict(variant_class);
    failable_variant execution = interpret_and_execute(code, "test", values, verbose, false);
    if (execution.failed) { failable_print(&execution); return false; }
    bool are_equal = variants_are_equal(execution.result, expected);
    if (!are_equal) {
        str_builder *expected_sb = new_str_builder();
        str_builder *exec_sb = new_str_builder();
        variant_describe(expected, expected_sb);
        variant_describe(execution.result, exec_sb);
        printf("Built in test case failed: \"%s\"\n", code);
        printf("  Expected: %s\n", str_builder_charptr(expected_sb));
        printf("  Resulted: %s\n", str_builder_charptr(exec_sb));
        str_builder_free(expected_sb);
        str_builder_free(exec_sb);
    }
    assert_msg(are_equal, code);
    return are_equal;
}

bool built_in_self_diagnostics(bool verbose) {
    bool all_passed = true;

    if (!validate_use_case(
        "[1,2,3,4].filter(function(item, idx, arr){return item % 2 == 0;})", 
        new_list_variant(list_of(variant_class, 2,
            new_int_variant(2),
            new_int_variant(4)
        )),
        verbose
    )) all_passed = false;

    if (!validate_use_case(
        "[1,2,3].map(function(item, idx, arr){return item * 2;})", 
        new_list_variant(list_of(variant_class, 3,
            new_int_variant(2),
            new_int_variant(4),
            new_int_variant(6)
        )),
        verbose
    )) all_passed = false;

    if (!validate_use_case(
        "[1,2,3].reduce(0, function(acc, item, idx, arr){return acc + item;})", 
        new_int_variant(6),
        verbose
    )) all_passed = false;

    return all_passed;
}

