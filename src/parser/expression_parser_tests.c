#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/str_builder.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../lexer/_module.h"
#include "expression_parser.h"


static bool use_case_passes(const char *code, bool expect_failure, expression *expected_expression, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    initialize_expression_parser(); // clean previous failed parsing attempts

    failable_list tokenization = parse_code_into_tokens(code, "test");
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
        str_builder *expected_sb = new_str_builder();
        str_builder *parsed_sb = new_str_builder();
        expression_describe(expected_expression, expected_sb);
        expression_describe(parsed, parsed_sb);
        fprintf(stderr, "Expression not as expected \"%s\", \n" \
                        "    expected: %s\n" \
                        "    parsed  : %s\n",
                        code, 
                        str_builder_charptr(expected_sb), 
                        str_builder_charptr(parsed_sb));
        str_builder_free(expected_sb);
        str_builder_free(parsed_sb);
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
        new_binary_expression(OP_ADD, NULL,
            new_identifier_expression("a", NULL),
            new_numeric_literal_expression("1", NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("1+2*3+4", false,
        new_binary_expression(OP_ADD, NULL, 
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_binary_expression(OP_MULTIPLY, NULL, 
                    new_numeric_literal_expression("2", NULL),
                    new_numeric_literal_expression("3", NULL)
                )
            ),
            new_numeric_literal_expression("4", NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("(1+2)*(3+4)", false,
        new_binary_expression(OP_MULTIPLY, NULL, 
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("3", NULL),
                new_numeric_literal_expression("4", NULL)
            )
        ), verbose)) all_passed = false;

    if (!use_case_passes("time(",    true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(,",   true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(1,",  true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("time(1,2", true, NULL, verbose)) all_passed = false;

    if (!use_case_passes("time()", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("time", NULL),
            new_list_data_expression(list_of(expression_class, 0), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14)", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("round", NULL),
            new_list_data_expression(list_of(expression_class, 1,
                new_numeric_literal_expression("3.14", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("round(3.14, 2)", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("round", NULL),
            new_list_data_expression(list_of(expression_class, 2,
                new_numeric_literal_expression("3.14", NULL),
                new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("pow(8, 2) + 1", false,
        new_binary_expression(OP_ADD, NULL,
            new_binary_expression(OP_FUNC_CALL, NULL,
                new_identifier_expression("pow", NULL),
                new_list_data_expression(list_of(expression_class, 2,
                    new_numeric_literal_expression("8", NULL),
                    new_numeric_literal_expression("2", NULL)
                ), NULL)
            ),
            new_numeric_literal_expression("1", NULL)
    ), verbose)) all_passed = false;

    if (!use_case_passes("a == 0", false,
        new_binary_expression(OP_EQUAL, NULL,
            new_identifier_expression("a", NULL),
            new_numeric_literal_expression("0", NULL)
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("iif(left(a, 1) == '0', 'number', 'letter')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("iif", NULL),
            new_list_data_expression(list_of(expression_class, 3, 
                new_binary_expression(OP_EQUAL, NULL, 
                    new_binary_expression(OP_FUNC_CALL, NULL,
                        new_identifier_expression("left", NULL),
                        new_list_data_expression(list_of(expression_class, 2, 
                            new_identifier_expression("a", NULL),
                            new_numeric_literal_expression("1", NULL)
                        ), NULL)),
                    new_string_literal_expression("0", NULL)),
                new_string_literal_expression("number", NULL),
                new_string_literal_expression("letter", NULL)
            ), NULL)
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("a ? b : c", false,
        new_binary_expression(OP_SHORT_IF, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 2,
                new_identifier_expression("b", NULL),
                new_identifier_expression("c", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("a > b ? c : d", false,
        new_binary_expression(OP_SHORT_IF, NULL,
            new_binary_expression(OP_GREATER_THAN, NULL,
                new_identifier_expression("a", NULL),
                new_identifier_expression("b", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 2,
                new_identifier_expression("c", NULL),
                new_identifier_expression("d", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("a ? b",     true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("a ? b , c", true, NULL, verbose)) all_passed = false;

    // although shunting yard is good for precedence, 
    // we need left-to-right association, not right-to-left
    // we want "1+2+3" => "(1+2)+3", not "1+(2+3)"
    if (!use_case_passes("1+2+3", false,
        new_binary_expression(OP_ADD, NULL,
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_numeric_literal_expression("3", NULL)
        ), verbose)) all_passed = false;

    // important difference: "(8-4)-2" = 2, while "8-(4-2)" = 6
    if (!use_case_passes("8-4-2", false,
        new_binary_expression(OP_SUBTRACT, NULL,
            new_binary_expression(OP_SUBTRACT, NULL, 
                new_numeric_literal_expression("8", NULL),
                new_numeric_literal_expression("4", NULL)
            ),
            new_numeric_literal_expression("2", NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("team.leader.name", false,
        new_binary_expression(OP_MEMBER, NULL,
            new_binary_expression(OP_MEMBER, NULL,
                new_identifier_expression("team", NULL),
                new_identifier_expression("leader", NULL)
            ),
            new_identifier_expression("name", NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("persons[2][3]", false,
        new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL,
            new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
                new_identifier_expression("persons", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_numeric_literal_expression("3", NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("person.children[2]", false,
        new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
            new_binary_expression(OP_MEMBER, NULL,
                new_identifier_expression("person", NULL),
                new_identifier_expression("children", NULL)
            ),
            new_numeric_literal_expression("2", NULL)
        ) , verbose)) all_passed = false;

    if (!use_case_passes("obj.method('hi')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_binary_expression(OP_MEMBER, NULL, 
                new_identifier_expression("obj", NULL),
                new_identifier_expression("method", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 1,
                new_string_literal_expression("hi", NULL)
            ), NULL)
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("methods[2]('hi')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
                new_identifier_expression("methods", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 1,
                new_string_literal_expression("hi", NULL)
            ), NULL)
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("handles[2].open('text')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_binary_expression(OP_MEMBER, NULL, 
                new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
                    new_identifier_expression("handles", NULL),
                    new_numeric_literal_expression("2", NULL)
                ),
                new_identifier_expression("open", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 1,
                new_string_literal_expression("text", NULL)
            ), NULL)
        ), verbose)) all_passed = false;
    
    if (!use_case_passes("a = [ 1, 2, 3 ]", false,
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 3,
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL),
                new_numeric_literal_expression("3", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("a = [ 1, 2, 3, ]", false, // notice extra comma
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 3,
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL),
                new_numeric_literal_expression("3", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("a = { key1:1, key2:2 }", false,
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_dict_data_expression(dict_of(expression_class, 2,
                "key1", new_numeric_literal_expression("1", NULL),
                "key2", new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    if (!use_case_passes("a = { key1:1, key2:2, }", false, // note extra comma
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_dict_data_expression(dict_of(expression_class, 2,
                "key1", new_numeric_literal_expression("1", NULL),
                "key2", new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose)) all_passed = false;

    return all_passed;
}
