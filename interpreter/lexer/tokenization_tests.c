#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../utils/failable.h"
#include "tokenization.h"



// for each token, pass in expected type. for identifiers and literals, pass in data.
static bool use_case_passes(const char *code, bool expect_failure, int expected_tokens, ...) {
    failable_list tokenization = parse_code_into_tokens(code, "test");

    // test failure
    if (expect_failure) {
        if (!tokenization.failed) {
            fprintf(stderr, "Tokenization did not fail as expected: (code=\"%s\")\n", code);
            return false;
        }
        return true;
    }

    // success, verify
    if (tokenization.failed) {
        fprintf(stderr, "Tokenization failed unexpectedly: %s\n\t(code=\"%s\")\n", tokenization.err_msg, code);
        return false;
    }

    list *tokens = tokenization.result;
    if (tokens == NULL) {
        fprintf(stderr, "Tokenization returned NULL list: code=\"%s\")\n", code);
        return false;
    }

    if (list_length(tokens) != expected_tokens) {
        fprintf(stderr, "Tokenization expected %d tokens, gotten %d, code=\"%s\")\n", expected_tokens, list_length(tokens), code);
        token_print_list(tokens, stderr, "    ", "\n");
        return false;
    }

    va_list args;
    va_start(args, expected_tokens);
    for (int i = 0; i < expected_tokens; i++) {
        token *t = list_get(tokens, i);

        token_type expected_type = va_arg(args, token_type);
        token_type actual_type = token_get_type(t);
        if (actual_type != expected_type) {
            fprintf(stderr, "Tokenization token #%d expected type %s, gotten %s, code=\"%s\")\n", 
                        i, token_type_str(expected_type), token_type_str(actual_type), code);
            return false;
        }
        if (actual_type == T_IDENTIFIER || actual_type == T_NUMBER_LITERAL || actual_type == T_STRING_LITERAL || actual_type == T_BOOLEAN_LITERAL) {
            char *expected_data = va_arg(args, char *);
            const char *actual_data = token_get_data(t);
            if (strcmp(actual_data, expected_data) != 0) {
                fprintf(stderr, "Tokenization token #%d expected data \"%s\", gotten \"%s\", code=\"%s\")\n", 
                            i, expected_data, actual_data, code);
                return false;
            }
        }
    }
    va_end(args);

    return true;
}

bool lexer_self_diagnostics() {
    bool all_passed = true;
    
    // for each token, pass in expected type. for identifiers and literals, pass in data.
    if (!use_case_passes("#", true, 0))
        all_passed = false;
    if (!use_case_passes(NULL, false, 1, T_END)) 
        all_passed = false;
    if (!use_case_passes("", false, 1, T_END))
        all_passed = false;
    if (!use_case_passes("index", false, 2, T_IDENTIFIER, "index", T_END))
        all_passed = false;
    if (!use_case_passes("123", false, 2, T_NUMBER_LITERAL, "123", T_END))
        all_passed = false;
    if (!use_case_passes("'hello'", false, 2, T_STRING_LITERAL, "hello", T_END))
        all_passed = false;
    if (!use_case_passes("\" abc \"", false, 2, T_STRING_LITERAL, " abc ", T_END))
        all_passed = false;
    if (!use_case_passes("true", false, 2, T_BOOLEAN_LITERAL, "true", T_END))
        all_passed = false;
    if (!use_case_passes("false", false, 2, T_BOOLEAN_LITERAL, "false", T_END))
        all_passed = false;
    if (!use_case_passes(">", false, 2, T_LARGER, T_END))
        all_passed = false;
    if (!use_case_passes(">=", false, 2, T_LARGER_EQUAL, T_END))
        all_passed = false;
    if (!use_case_passes(">>", false, 2, T_DOUBLE_LARGER, T_END))
        all_passed = false;
    if (!use_case_passes("iif(a >= 10, b, c)", false, 11, 
        T_IDENTIFIER, "iif",
        T_LPAREN,
        T_IDENTIFIER, "a",
        T_LARGER_EQUAL,
        T_NUMBER_LITERAL, "10",
        T_COMMA,
        T_IDENTIFIER, "b",
        T_COMMA,
        T_IDENTIFIER, "c",
        T_RPAREN,
        T_END))
        all_passed = false;

    return all_passed;
}
