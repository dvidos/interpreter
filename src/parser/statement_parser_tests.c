#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/testing.h"
#include "../utils/str_builder.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../lexer/_module.h"
#include "../entities/expression.h"
#include "../entities/statement.h"
#include "expression_parser.h"
#include "statement_parser.h"


static void run_use_case(const char *code, bool expect_failure, statement *expected_statement, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    failable_list tokenization = parse_code_into_tokens(code, "test");
    if (tokenization.failed) {
        assertion_failed(tokenization.err_msg, code);
        return;
    }

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);
    failable_statement parsing = parse_statement(tokens_it);
    
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

        assert_statements_are_equal(parsing.result, expected_statement, code);
    }
}

void statement_parser_self_diagnostics(bool verbose) {
    run_use_case("if (a) b;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 1, 
                new_expression_statement(new_identifier_expression("b", NULL))),
            false,
            NULL
        ), 
    verbose);

    run_use_case("if (a) return a;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 1, 
                new_return_statement(new_identifier_expression("a", NULL))
            ),
            false,
            NULL
        ), 
    verbose);

    run_use_case("if (a) { b; c; }", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 2,
                new_expression_statement(new_identifier_expression("b", NULL)),
                new_expression_statement(new_identifier_expression("c", NULL))),
            false,
            NULL
        ), 
    verbose);

    run_use_case("if (a) b; else c;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 1, new_expression_statement(new_identifier_expression("b", NULL))),
            true,
            list_of(statement_class, 1, new_expression_statement(new_identifier_expression("c", NULL)))
        ), 
    verbose);

    run_use_case("if (a) { b; c; } else { d; e; }", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 2, 
                new_expression_statement(new_identifier_expression("b", NULL)),
                new_expression_statement(new_identifier_expression("c", NULL))),
            true,
            list_of(statement_class, 2, 
                new_expression_statement(new_identifier_expression("d", NULL)), 
                new_expression_statement(new_identifier_expression("e", NULL)))
        ), 
    verbose);

    run_use_case("while (a) b;", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 1, new_expression_statement(new_identifier_expression("b", NULL)))
        ), 
    verbose);

    run_use_case("while (a) { b; c; }", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 2, 
                new_expression_statement(new_identifier_expression("b", NULL)), 
                new_expression_statement(new_identifier_expression("c", NULL)))
        ), 
    verbose);

    run_use_case("while (a) { break; continue; }", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(statement_class, 2, 
                new_break_statement(), 
                new_continue_statement())
        ), 
    verbose);

    run_use_case("for(a; b; c) d;", false, 
        new_for_statement(
            new_identifier_expression("a", NULL),
            new_identifier_expression("b", NULL),
            new_identifier_expression("c", NULL),
            list_of(statement_class, 1, new_expression_statement(new_identifier_expression("d", NULL)))
        ), 
    verbose);

    run_use_case("for(a; b; c) { d; e; }", false, 
        new_for_statement(
            new_identifier_expression("a", NULL),
            new_identifier_expression("b", NULL),
            new_identifier_expression("c", NULL),
            list_of(statement_class, 2, 
                new_expression_statement(new_identifier_expression("d", NULL)),
                new_expression_statement(new_identifier_expression("e", NULL)))
        ), 
    verbose);

    run_use_case("for (a) { c; d; }", true, NULL, verbose);
    run_use_case("for (a;b) { c; d; }", true, NULL, verbose);
    // run_use_case("for (a;b;c;d) { e; f; }", true, NULL, verbose);
}
