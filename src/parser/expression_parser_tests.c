#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/testing.h"
#include "../utils/str_builder.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../lexer/_module.h"
#include "expression_parser.h"


static void run_use_case(const char *code, bool expect_failure, expression *expected_expression, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    initialize_expression_parser(); // clean previous failed parsing attempts

    failable_list tokenization = parse_code_into_tokens(code, "test");
    if (tokenization.failed) {
        assertion_failed(tokenization.err_msg, code);
        return;
    }

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);
    failable_expression parsing = parse_expression(tokens_it, CM_END_OF_TEXT, verbose);

    if (expect_failure) {
        if (!parsing.failed) {
            assertion_failed("Parsing did not fail as expected", code);
            return;
        }

    } else {
        if (parsing.failed) {
            assertion_failed(parsing.err_msg, code);
            return;
        }

        assert_expressions_are_equal(parsing.result, expected_expression, code);
    }

}

void expression_parser_self_diagnostics(bool verbose) {

    run_use_case(NULL, true, NULL, verbose);
    run_use_case("",   true, NULL, verbose);
    run_use_case("+",  true, NULL, verbose);
    run_use_case("a+", true, NULL, verbose);

    run_use_case("a+1", false,
        new_binary_expression(OP_ADD, NULL,
            new_identifier_expression("a", NULL),
            new_numeric_literal_expression("1", NULL)
        ), verbose);

    run_use_case("1+2*3+4", false,
        new_binary_expression(OP_ADD, NULL, 
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_binary_expression(OP_MULTIPLY, NULL, 
                    new_numeric_literal_expression("2", NULL),
                    new_numeric_literal_expression("3", NULL)
                )
            ),
            new_numeric_literal_expression("4", NULL)
        ), verbose);
    run_use_case("(1+2)*(3+4)", false,
        new_binary_expression(OP_MULTIPLY, NULL, 
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("3", NULL),
                new_numeric_literal_expression("4", NULL)
            )
        ), verbose);

    run_use_case("time(",    true, NULL, verbose);
    run_use_case("time(,",   true, NULL, verbose);
    run_use_case("time(1,",  true, NULL, verbose);
    run_use_case("time(1,2", true, NULL, verbose);

    run_use_case("time()", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("time", NULL),
            new_list_data_expression(list_of(expression_class, 0), NULL)
        ), verbose);

    run_use_case("round(3.14)", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("round", NULL),
            new_list_data_expression(list_of(expression_class, 1,
                new_numeric_literal_expression("3.14", NULL)
            ), NULL)
        ), verbose);

    run_use_case("round(3.14, 2)", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_identifier_expression("round", NULL),
            new_list_data_expression(list_of(expression_class, 2,
                new_numeric_literal_expression("3.14", NULL),
                new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose);

    run_use_case("pow(8, 2) + 1", false,
        new_binary_expression(OP_ADD, NULL,
            new_binary_expression(OP_FUNC_CALL, NULL,
                new_identifier_expression("pow", NULL),
                new_list_data_expression(list_of(expression_class, 2,
                    new_numeric_literal_expression("8", NULL),
                    new_numeric_literal_expression("2", NULL)
                ), NULL)
            ),
            new_numeric_literal_expression("1", NULL)
    ), verbose);

    run_use_case("a == 0", false,
        new_binary_expression(OP_EQUAL, NULL,
            new_identifier_expression("a", NULL),
            new_numeric_literal_expression("0", NULL)
        ), verbose);
    
    run_use_case("iif(left(a, 1) == '0', 'number', 'letter')", false,
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
        ), verbose);
    
    run_use_case("a ? b : c", false,
        new_binary_expression(OP_SHORT_IF, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 2,
                new_identifier_expression("b", NULL),
                new_identifier_expression("c", NULL)
            ), NULL)
        ), verbose);

    run_use_case("a > b ? c : d", false,
        new_binary_expression(OP_SHORT_IF, NULL,
            new_binary_expression(OP_GREATER_THAN, NULL,
                new_identifier_expression("a", NULL),
                new_identifier_expression("b", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 2,
                new_identifier_expression("c", NULL),
                new_identifier_expression("d", NULL)
            ), NULL)
        ), verbose);

    run_use_case("a ? b",     true, NULL, verbose);
    run_use_case("a ? b , c", true, NULL, verbose);

    // although shunting yard is good for precedence, 
    // we need left-to-right association, not right-to-left
    // we want "1+2+3" => "(1+2)+3", not "1+(2+3)"
    run_use_case("1+2+3", false,
        new_binary_expression(OP_ADD, NULL,
            new_binary_expression(OP_ADD, NULL, 
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_numeric_literal_expression("3", NULL)
        ), verbose);

    // important difference: "(8-4)-2" = 2, while "8-(4-2)" = 6
    run_use_case("8-4-2", false,
        new_binary_expression(OP_SUBTRACT, NULL,
            new_binary_expression(OP_SUBTRACT, NULL, 
                new_numeric_literal_expression("8", NULL),
                new_numeric_literal_expression("4", NULL)
            ),
            new_numeric_literal_expression("2", NULL)
        ), verbose);

    run_use_case("team.leader.name", false,
        new_binary_expression(OP_MEMBER, NULL,
            new_binary_expression(OP_MEMBER, NULL,
                new_identifier_expression("team", NULL),
                new_identifier_expression("leader", NULL)
            ),
            new_identifier_expression("name", NULL)
        ), verbose);

    run_use_case("persons[2][3]", false,
        new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL,
            new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
                new_identifier_expression("persons", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_numeric_literal_expression("3", NULL)
        ), verbose);

    run_use_case("person.children[2]", false,
        new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
            new_binary_expression(OP_MEMBER, NULL,
                new_identifier_expression("person", NULL),
                new_identifier_expression("children", NULL)
            ),
            new_numeric_literal_expression("2", NULL)
        ) , verbose);

    run_use_case("obj.method('hi')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_binary_expression(OP_MEMBER, NULL, 
                new_identifier_expression("obj", NULL),
                new_identifier_expression("method", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 1,
                new_string_literal_expression("hi", NULL)
            ), NULL)
        ), verbose);
    
    run_use_case("methods[2]('hi')", false,
        new_binary_expression(OP_FUNC_CALL, NULL,
            new_binary_expression(OP_ARRAY_SUBSCRIPT, NULL, 
                new_identifier_expression("methods", NULL),
                new_numeric_literal_expression("2", NULL)
            ),
            new_list_data_expression(list_of(expression_class, 1,
                new_string_literal_expression("hi", NULL)
            ), NULL)
        ), verbose);
    
    run_use_case("handles[2].open('text')", false,
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
        ), verbose);
    
    run_use_case("a = [ 1, 2, 3 ]", false,
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 3,
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL),
                new_numeric_literal_expression("3", NULL)
            ), NULL)
        ), verbose);

    run_use_case("a = [ 1, 2, 3, ]", false, // notice extra comma
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_list_data_expression(list_of(expression_class, 3,
                new_numeric_literal_expression("1", NULL),
                new_numeric_literal_expression("2", NULL),
                new_numeric_literal_expression("3", NULL)
            ), NULL)
        ), verbose);

    run_use_case("a = { key1:1, key2:2 }", false,
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_dict_data_expression(dict_of(expression_class, 2,
                "key1", new_numeric_literal_expression("1", NULL),
                "key2", new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose);

    run_use_case("a = { key1:1, key2:2, }", false, // note extra comma
        new_binary_expression(OP_ASSIGNMENT, NULL,
            new_identifier_expression("a", NULL),
            new_dict_data_expression(dict_of(expression_class, 2,
                "key1", new_numeric_literal_expression("1", NULL),
                "key2", new_numeric_literal_expression("2", NULL)
            ), NULL)
        ), verbose);
}
