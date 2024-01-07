#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/failable.h"
#include "../utils/list.h"
#include "../utils/stack.h"
#include "expression.h"
#include "token.h"
#include "tokenization.h"
#include "parser.h"
#include "eval.h"
#include "execution.h"


void initialize_evaluator() {
    initialize_char_tokens_trie();
    initialize_operator_tables();
}


failable_value evaluate(const char *code, dict *arguments) {

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed)
        return failed_value("Tokenization failed: %s", tokenization.err_msg);
    
    failable_list parsing = parse_tokens_into_expressions(tokenization.result, false);
    if (parsing.failed)
        return failed_value("Parsing failed: %s", parsing.err_msg);

    value *result = new_value();
    for (sequential *s = list_sequential(parsing.result); s != NULL; s = s->next) {
        expression *expr = (expression *)s->data;
        failable_value execution = execute_expression(expr, arguments);
        if (execution.failed)
            return failed_value("Execution failed: %s", execution.err_msg);
        
        // if many expressions, the last status is kept and returned.
        result = execution.result;
    }

    return ok_value(result);
}
