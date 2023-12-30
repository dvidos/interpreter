#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/status.h"
#include "../utils/list.h"
#include "../utils/stack.h"
#include "expression.h"
#include "token.h"
#include "tokenization.h"
#include "parser.h"
#include "eval.h"


status evaluate(const char *code, dict *arguments, value **result_out) {
    status s;

    list *tokens;
    s = parse_code_into_tokens(code, &tokens);
    if (s.failed)
        return s;
    
    list *expressions;
    s = parse_tokens_into_expressions(tokens, &expressions);
    if (s.failed)
        return s;

    *result_out = new_null_value();
    for (int i = 0; i < list_length(expressions); i++) {
        expression *expr = (expression *)list_get(expressions, i);

        // if many expressions, the last status is returned.
        s = execute_expression(expr, arguments, result_out);
        if (s.failed)
            return s;
    }

    return status_ok();
}
