#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/strbld.h"
#include "../../utils/failable.h"
#include "../../utils/containers/list.h"
#include "../../utils/containers/stack.h"
#include "../lexer/token.h"
#include "../lexer/tokenization.h"
#include "parser.h"
#include "operator.h"
#include "expression.h"


static bool use_case_passes(const char *code, bool expect_failure, list *expected_expressions, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    failable_list tokenization = parse_code_into_tokens(code);
    if (tokenization.failed) {
        fprintf(stderr, "Parsing tokenization failed unexpectedly: %s\n\t(code=\"%s\")", tokenization.err_msg, code);
        return false;
    }

    failable_list parsing = parse_tokens_into_expressions(tokenization.result, verbose);

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

    // compare lengths first
    list *parsed_expressions = parsing.result;
    if (list_length(parsed_expressions) != list_length(expected_expressions)) {
        fprintf(stderr, "Expected %d expressions, gotten %d, (code=\"%s\")\n", 
            list_length(expected_expressions), list_length(parsed_expressions), code);
        fprintf(stderr, "    %s\n", list_to_string(parsed_expressions, "\n    "));
        return false;
    }

    // compare each expression
    for (int i = 0; i < list_length(expected_expressions); i++) {
        expression *parsed = list_get(parsed_expressions, i);
        expression *expected = list_get(expected_expressions, i);
        if (!expressions_are_equal(parsed, expected)) {
            fprintf(stderr, "Expression #%d differs, (code=\"%s\"), \n" \
                            "  expected: %s\n" \
                            "  parsed:   %s\n",
                            i, code, expression_to_string(expected), expression_to_string(parsed));
            return false;
        }
    }

    return true;
}

// perform unit tests and report if successful
bool parser_self_diagnostics(bool verbose) {
    bool all_passed = true;

    if (!use_case_passes(NULL, false, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("",   false, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("+",  true,  list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("a+", true,  list_of(containing_expressions, 0), verbose)) all_passed = false;

    if (!use_case_passes("a+1", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_ADD, 
            new_identifier_expression("a"),
            new_numeric_literal_expression("1")
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("1+2*3+4", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_ADD, 
            new_numeric_literal_expression("1"),
            new_binary_op_expression(OP_ADD, 
                new_binary_op_expression(OP_MULTIPLY, 
                    new_numeric_literal_expression("2"),
                    new_numeric_literal_expression("3")
                ),
                new_numeric_literal_expression("4")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("(1+2)*(3+4)", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_MULTIPLY, 
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("1"),
                new_numeric_literal_expression("2")
            ),
            new_binary_op_expression(OP_ADD, 
                new_numeric_literal_expression("3"),
                new_numeric_literal_expression("4")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("time(",    true, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("time(,",   true, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("time(1,",  true, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("time(1,2", true, list_of(containing_expressions, 0), verbose)) all_passed = false;

    if (!use_case_passes("time()", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("time"),
            new_func_args_expression(list_of(containing_expressions, 0))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14)", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(containing_expressions, 1,
                new_numeric_literal_expression("3.14")
            ))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14, 2)", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("round"),
            new_func_args_expression(list_of(containing_expressions, 2,
                new_numeric_literal_expression("3.14"),
                new_numeric_literal_expression("2")
            ))
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("pow(8, 2) + 1", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_ADD,
            new_binary_op_expression(OP_FUNC_CALL,
                new_identifier_expression("pow"),
                new_func_args_expression(list_of(containing_expressions, 2,
                    new_numeric_literal_expression("8"),
                    new_numeric_literal_expression("2")
                ))
            ),
            new_numeric_literal_expression("1")
    )), verbose)) all_passed = false;

    if (!use_case_passes("a == 0", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_EQUAL,
            new_identifier_expression("a"),
            new_numeric_literal_expression("0")
        )
    ), verbose)) all_passed = false;
    
    if (!use_case_passes("if(left(a, 1) == '0', 'number', 'letter')", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_FUNC_CALL,
            new_identifier_expression("if"),
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
        )
    ), verbose)) all_passed = false;
    
    if (!use_case_passes("a ? b : c", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_SHORT_IF,
            new_identifier_expression("a"),
            new_pair_expression(
                new_identifier_expression("b"),
                new_identifier_expression("c")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("a > b ? c : d", false, list_of(containing_expressions, 1,
        new_binary_op_expression(OP_SHORT_IF,
            new_binary_op_expression(OP_GREATER_THAN,
                new_identifier_expression("a"),
                new_identifier_expression("b")
            ),
            new_pair_expression(
                new_identifier_expression("c"),
                new_identifier_expression("d")
            )
        )
    ), verbose)) all_passed = false;

    if (!use_case_passes("a ? b", true, list_of(containing_expressions, 0), verbose)) all_passed = false;
    if (!use_case_passes("a ? b , c", true, list_of(containing_expressions, 0), verbose)) all_passed = false;
    
    return all_passed;
}
