#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/testing.h"
#include "../utils/failable.h"
#include "../utils/containers/list.h"
#include "../utils/containers/stack.h"
#include "parser/expression.h"
#include "parser/parser.h"
#include "lexer/token.h"
#include "lexer/tokenization.h"
#include "runtime/built_in_funcs.h"
#include "runtime/execution.h"
#include "interpreter.h"


void initialize_interpreter() {
    initialize_built_in_funcs_table();
    initialize_char_tokens_trie();
    initialize_operator_tables();
}


failable_variant interpret_and_execute(const char *code, dict *arguments) {

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed)
        return failed("Tokenization failed: %s", tokenization.err_msg);
    
    failable_list parsing = parse_tokens_into_expressions(tokenization.result, false);
    if (parsing.failed)
        return failed("Parsing failed: %s", parsing.err_msg);

    variant *result = new_null_variant();
    for_list(parsing.result, results_iterator, expression, expr) {
        failable_variant execution = execute_expression(expr, arguments, get_built_in_funcs_table());
        if (execution.failed)
            return failed("Execution failed: %s", execution.err_msg);
        
        // if many expressions, the last result is kept and returned.
        result = execution.result;
    }

    return ok_variant(result);
}
