#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/str_builder.h"
#include "../../utils/failable.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/stack.h"
#include "../lexer/token.h"
#include "../lexer/tokenization.h"
#include "expression_parser.h"
#include "operator.h"
#include "expression.h"


static bool use_case_passes(const char *code, bool expect_failure, expression *expected_expression, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed) {
        fprintf(stderr, "Parsing tokenization failed unexpectedly: %s\n\t(code=\"%s\")", tokenization.err_msg, code);
        return false;
    }

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);

    failable_expression parsing = parse_expression(tokens_it, CM_END_OF_TEXT, verbose);

    // test failure
    if (expect_failure) {
        if (!parsing.failed) {
            fprintf(stderr, "Parsing did not fail as expected: (code=\"%s\")\n", code);
            return false;
        }
        return true;
    }

    // success, verify
    if (parsing.failed) {
        fprintf(stderr, "Parsing failed unexpectedly: %s\n\t(code=\"%s\")\n", parsing.err_msg, code);
        return false;
    }

    // compare each expression
    expression *parsed = parsing.result;
    if (!expressions_are_equal(parsed, expected_expression)) {
        fprintf(stderr, "Expression differs, (code=\"%s\"), \n" \
                        "  expected: %s\n" \
                        "  parsed:   %s\n",
                        code, expression_to_string(expected_expression), expression_to_string(parsed));
        return false;
    }

    return true;
}

// perform unit tests and report if successful
bool expression_parser_self_diagnostics(bool verbose) {
    bool all_passed = true;

    if (!use_case_passes(NULL, true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("",   true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("+",  true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("a+", true, NULL, verbose)) all_passed = false;

    if (!use_case_passes("a+1", false,
        new_binary_op_expression(OP_ADD, 
            new_identifier_expression("a"),
            new_numeric_literal_expression("1")
        ), verbose)) all_passed = false;

    if (!use_case_passes("1+2*3+4", false,
        new_binary_op_expression(OP_ADD, 
            new_numeric_literal_expression("1"),
            new_binary_op_expression(OP_ADD, 
                new_binary_op_expression(OP_MULTIPLY, 
                    new_numeric_literal_expression("2"),
                    new_numeric_literal_expression("3")
                ),
                new_numeric_literal_expression("4")
            )
        ), verbose)) all_passed = false;

    if (!use_case_passes("(1+2)*(3+4)", false,
        new_binary_op_expression(OP_MULTIPLY, 
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("1"),
                new_numeric_literal_expression("2")
            ),
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("3"),
                new_numeric_literal_expression("4")
            )
        ), verbose)) all_passed = false;

    if (!use_case_passes("time(",    true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(,",   true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(1,",  true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(1,2", true, NULL, verbose)) all_passed = false;

    if (!use_case_passes("time()", false,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("time"),
            new_func_args_expression(list_of(containing_expressions, 0))
        ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14)", false,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(containing_expressions, 1,
                new_numeric_literal_expression("3.14")
            ))
        ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14, 2)", false,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(containing_expressions, 2,
                new_numeric_literal_expression("3.14"),
                new_numeric_literal_expression("2")
            ))
        ), verbose)) all_passed = false;

    if (!use_case_passes("pow(8, 2) + 1", false,
        new_binary_op_expression(OP_ADD,
            new_binary_op_expression(OP_FUNC_CALL,
                new_identifier_expression("pow"),
                new_func_args_expression(list_of(containing_expressions, 2,
                    new_numeric_literal_expression("8"),
                    new_numeric_literal_expression("2")
                ))
            ),
            new_numeric_literal_expression("1")
    ), verbose)) all_passed = false;

    if (!use_case_passes("a == 0", false,
        new_binary_op_expression(OP_EQUAL,
            new_identifier_expression("a"),
            new_numeric_literal_expression("0")
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("iif(left(a, 1) == '0', 'number', 'letter')", false,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("iif"),
            new_func_args_expression(list_of(containing_expressions, 3, 
                new_binary_op_expression(OP_EQUAL, 
                    new_binary_op_expression(OP_FUNC_CALL,
                        new_identifier_expression("left"),
                        new_func_args_expression(list_of(containing_expressions, 2, 
                            new_identifier_expression("a"),
                            new_numeric_literal_expression("1")
                        ))),
                    new_string_literal_expression("0")),
                new_string_literal_expression("number"),
                new_string_literal_expression("letter")
            ))
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("a ? b : c", false,
        new_binary_op_expression(OP_SHORT_IF,
            new_identifier_expression("a"),
            new_pair_expression(
                new_identifier_expression("b"),
                new_identifier_expression("c")
            )
        ), verbose)) all_passed = false;

    if (!use_case_passes("a > b ? c : d", false,
        new_binary_op_expression(OP_SHORT_IF,
            new_binary_op_expression(OP_GREATER_THAN,
                new_identifier_expression("a"),
                new_identifier_expression("b")
            ),
            new_pair_expression(
                new_identifier_expression("c"),
                new_identifier_expression("d")
            )
        ), verbose)) all_passed = false;

    if (!use_case_passes("a ? b",     true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("a ? b , c", true, NULL, verbose)) all_passed = false;
    
    return all_passed;
}
