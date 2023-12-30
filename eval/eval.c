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


failable_value evaluate(const char *code, dict *arguments) {

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed)
        return failed_value("Tokenization failed: %s", tokenization.err_msg);
    
    failable_list parsing = parse_tokens_into_expressions(tokenization.result);
    if (parsing.failed)
        return failed_value("Parsing failed: %s", parsing.err_msg);

    value *result = new_null_value();
    for (iterator *it = list_iterator(parsing.result); iterator_valid(it); iterator_next(it)) {
        expression *expr = (expression *)iterator_current(it);
        failable_value execution = execute_expression(expr, arguments);
        if (execution.failed)
            return failed_value("Execution failed: %s", execution.err_msg);
        
        // if many expressions, the last status is kept and returned.
        result = execution.result;
    }

    return ok_value(result);
}
