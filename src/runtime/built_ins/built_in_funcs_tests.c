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
    else if (ex.excepted)
        assertion_failed(str_variant_as_str(variant_to_string(ex.exception_thrown)), code);
    else
        assert_variants_are_equal(ex.result, expected, code);
}

void built_in_self_diagnostics(bool verbose) {
}

