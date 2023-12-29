#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "list.h"
#include "stack.h"
#include "expression.h"
#include "token.h"
#include "tokenization.h"
#include "eval.h"


static list *parse_tokens_into_expressions(list *tokens) {
    // parse the tokens into an AST. Need operation precedence here & double stack.
    return new_list();
}

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
