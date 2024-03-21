#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/cstr.h"
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../containers/_containers.h"
#include "../entities/_entities.h"
#include "../lexer/_lexer.h"
#include "../parser/_parser.h"
#include "../runtime/_runtime.h"
#include "../runtime/built_ins/built_in_funcs.h"
#include "../runtime/execution/expression_execution.h"
#include "../runtime/execution/statement_execution.h"
#include "interpreter.h"


void initialize_interpreter() {
    initialize_lexer();
    initialize_operator_type_tables();
    initialize_expression_parser();
    initialize_statement_parser();

    initialize_variants();
    initialize_built_in_funcs_table();
    initialize_expression_execution();
}


execution_outcome interpret_and_execute(const char *code, const char *filename, dict *external_values, bool verbose, bool enable_debugger, bool start_with_debugger) {
    str_builder *sb = new_str_builder();

    listing *code_listing = new_listing(code);

    failable_list tokenization = parse_code_into_tokens(code, filename);
    if (tokenization.failed) {
        failable_print(&tokenization);
        return failed_outcome("Tokenization failed");
    }
    if (verbose) {
        str_builder_clear(sb);
        list_describe(tokenization.result, ", ", sb);
        printf("------------- parsed tokens -------------\n%s\n", str_builder_charptr(sb));
    }

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);
    failable_list parsing = parse_statements(tokens_it, SP_SEQUENTIAL_STATEMENTS);
    if (parsing.failed) {
        failable_print(&parsing);
        return failed_outcome("Statement parsing failed");
    }
    if (verbose) {
        str_builder_clear(sb);
        list_describe(parsing.result, "\n", sb);
        printf("------------- parsed statements -------------\n%s\n", str_builder_charptr(sb));
    }

    exec_context *ctx = new_exec_context(filename, code_listing, parsing.result, external_values, verbose, enable_debugger, start_with_debugger);
    dict *built_ins = get_built_in_funcs_table();
    for_dict(built_ins, bi_it, cstr, bltin_name)
        exec_context_register_built_in(ctx, bltin_name, new_callable_variant(dict_get(built_ins, bltin_name)));
    exec_context_log_reset();


    if (verbose)
        printf("------------- executing -------------\n");
    execution_outcome execution = execute_statements(parsing.result, ctx);

    // no matter exception, failure, or sucess.
    return execution;
}
