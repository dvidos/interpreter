#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "built_in_funcs_tests.h"
#include "../../interpreter/interpreter.h"
#include "../../utils/data_types/_module.h"
#include "../../utils/testing.h"


static void run_use_case(const char *code, variant *expected, bool verbose) {
    dict *values = new_dict(variant_item_info);
    execution_outcome ex = interpret_and_execute(code, "test", values, verbose, false, false);
    if (ex.failed)
        assertion_failed(ex.failure_message, code);
    else if (ex.exception_thrown)
        assertion_failed(str_variant_as_str(variant_to_string(ex.exception)), code);
    else
        assert_variants_are_equal(ex.result, expected, code);
}

void built_in_self_diagnostics(bool verbose) {

    run_use_case(
        "[1,2,3,4].filter(function(item, idx, arr){return item % 2 == 0;})", 
        new_list_variant_of(2,
            new_int_variant(2),
            new_int_variant(4)
        ),
        verbose
    );

    run_use_case(
        "[1,2,3].map(function(item, idx, arr){return item * 2;})", 
        new_list_variant_of(3,
            new_int_variant(2),
            new_int_variant(4),
            new_int_variant(6)
        ),
        verbose
    );

    run_use_case(
        "[1,2,3].reduce(0, function(acc, item, idx, arr){return acc + item;})", 
        new_int_variant(6),
        verbose
    );

}

