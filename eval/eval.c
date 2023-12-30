#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/list.h"
#include "../utils/stack.h"
#include "expression.h"
#include "token.h"
#include "tokenization.h"
#include "parser.h"
#include "eval.h"


value *evaluate(const char *code, dict *arguments) {
    list *tokens = parse_code_into_tokens(code);
    list *expressions = parse_tokens_into_expressions(tokens);

    value *result = new_null_value();
    for (int i = 0; i < list_length(expressions); i++) {
        expression *expr = (expression *)list_get(expressions, i);

        // if many expressions, the last result is returned.
        result = execute_expression(expr, arguments);
    }

    return result;
}
