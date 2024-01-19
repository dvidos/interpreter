#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "entities/_module.h"
#include "lexer/_module.h"
#include "parser/_module.h"
#include "runtime/_module.h"

#include "runtime/built_in_funcs.h"
#include "runtime/expression_execution.h"
#include "runtime/statement_execution.h"
#include "runtime/exec_context.h"
#include "interpreter.h"


void initialize_interpreter() {
    initialize_lexer();
    initialize_operator_tables();
    initialize_expression_parser();
    initialize_statement_parser();
    initialize_built_in_funcs_table();
}


failable_variant interpret_and_execute(const char *code, dict *arguments, bool verbose) {

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed)
        return failed_variant("Tokenization failed: %s", tokenization.err_msg);
    if (verbose)
        printf("Parsed tokens: %s\n", list_to_string(tokenization.result, ", "));

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);

    failable_list parsing = parse_statements(tokens_it, false);
    if (parsing.failed)
        return failed_variant("Statement parsing failed: %s", parsing.err_msg);
    if (verbose)
        printf("Parsed statements: %s\n", list_to_string(parsing.result, "\n"));

    exec_context_log_reset();

    failable_variant execution = execute_statements(parsing.result, arguments, get_built_in_funcs_table());
    if (execution.failed)
        return failed_variant("Execution failed: %s", execution.err_msg);

    return ok_variant(execution.result);
}
