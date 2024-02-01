#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../entities/_module.h"
#include "../lexer/_module.h"
#include "../parser/_module.h"
#include "../runtime/_module.h"

#include "../runtime/built_in_funcs.h"
#include "../runtime/expression_execution.h"
#include "../runtime/statement_execution.h"
#include "../runtime/exec_context.h"
#include "interpreter.h"


void initialize_interpreter() {
    initialize_lexer();
    initialize_operator_type_tables();
    initialize_expression_parser();
    initialize_statement_parser();
    initialize_built_in_funcs_table();
    initialize_expression_execution();
}


failable_variant interpret_and_execute(const char *code, const char *filename, dict *external_values, bool verbose) {

    failable_list tokenization = parse_code_into_tokens(code, filename);
    if (tokenization.failed)
        return failed_variant(&tokenization, "Tokenization failed");
    if (verbose)
        printf("------------- parsed tokens -------------\n%s\n", list_to_string(tokenization.result, ", "));

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);
    failable_list parsing = parse_statements(tokens_it, SP_SEQUENTIAL_STATEMENTS);
    if (parsing.failed)
        return failed_variant(&parsing, "Statement parsing failed");
    if (verbose)
        printf("------------- parsed statements -------------\n%s\n", list_to_string(parsing.result, "\n"));

    exec_context *ctx = new_exec_context(verbose);
    dict *built_ins = get_built_in_funcs_table();
    for_dict(external_values, evit, str, var_name)
        register_symbol(ctx->symbols, var_name, dict_get(external_values, var_name));
    for_dict(built_ins, biit, str, bi_name)
        register_symbol(ctx->symbols, bi_name, new_callable_variant(dict_get(built_ins, bi_name)));
    exec_context_log_reset();

    if (verbose)
        printf("------------- executing -------------\n");
    failable_variant execution = execute_statements(parsing.result, ctx);
    if (execution.failed)
        return failed_variant(&execution, "Execution failed");

    return ok_variant(execution.result);
}
