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
    list *tokens = tokenization.result;
    
    failable_list parsing = parse_tokens_into_expressions(tokens);
    if (parsing.failed)
        return failed_value("Parsing failed: %s", parsing.err_msg);
    list *expressions = parsing.result;

    value *result = new_null_value();
    for (int i = 0; i < list_length(expressions); i++) {
        expression *expr = (expression *)list_get(expressions, i);

        // if many expressions, the last status is kept and returned.
        failable_value execution = execute_expression(expr, arguments);
        if (execution.failed)
            return failed_value("Execution failed: %s", execution.err_msg);
        result = execution.result;
    }

    return ok_value(result);
}
