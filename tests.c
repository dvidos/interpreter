#include "utils/testing.h"
#include "eval/tokenization.h"
#include "eval/parser.h"
#include "eval/eval.h"

bool run_unit_tests() {

    tokenizer_self_diagnostics();
    parser_self_diagnostics(false);
    evaluator_self_diagnostics();

    return testing_outcome(true);
}
