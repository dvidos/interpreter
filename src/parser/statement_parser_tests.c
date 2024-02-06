#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../utils/str_builder.h"
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../lexer/_module.h"
#include "../entities/expression.h"
#include "../entities/statement.h"
#include "expression_parser.h"
#include "statement_parser.h"


static bool use_case_passes(const char *code, bool expect_failure, statement *expected_statement, bool verbose) {
    if (verbose)
        fprintf(stderr, "---------- use case: \"%s\" ----------\n", code);

    failable_list tokenization = parse_code_into_tokens(code, "test");
    if (tokenization.failed) {
        fprintf(stderr, "Parsing tokenization failed unexpectedly: %s\n\t(code=\"%s\")", tokenization.err_msg, code);
        return false;
    }
    // printf("%s\n", list_describe(tokenization.result, ", "));

    iterator *tokens_it = list_iterator(tokenization.result);
    tokens_it->reset(tokens_it);

    failable_statement parsing = parse_statement(tokens_it);
    
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
    statement *parsed = parsing.result;
    if (!statements_are_equal(parsed, expected_statement)) {
        str_builder *expected_sb = new_str_builder();
        str_builder *parsed_sb = new_str_builder();
        statement_describe(expected_statement, expected_sb);
        statement_describe(parsed, parsed_sb);
        fprintf(stderr, "Statement differs, (code=\"%s\"), \n" \
                        "  expected: %s\n" \
                        "  parsed:   %s\n",
                        code, 
                        str_builder_charptr(expected_sb), 
                        str_builder_charptr(parsed_sb));
        str_builder_free(expected_sb);
        str_builder_free(parsed_sb);
        return false;
    }

    return true;
}

bool statement_parser_self_diagnostics(bool verbose) {
    bool all_passed = true;
    
    if (!use_case_passes("if (a) b;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 1, 
                new_expression_statement(new_identifier_expression("b", NULL))),
            false,
            NULL
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("if (a) return a;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 1, 
                new_return_statement(new_identifier_expression("a", NULL))
            ),
            false,
            NULL
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("if (a) { b; c; }", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 2,
                new_expression_statement(new_identifier_expression("b", NULL)),
                new_expression_statement(new_identifier_expression("c", NULL))),
            false,
            NULL
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("if (a) b; else c;", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 1, new_expression_statement(new_identifier_expression("b", NULL))),
            true,
            list_of(containing_statements, 1, new_expression_statement(new_identifier_expression("c", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("if (a) { b; c; } else { d; e; }", false, 
        new_if_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 2, 
                new_expression_statement(new_identifier_expression("b", NULL)),
                new_expression_statement(new_identifier_expression("c", NULL))),
            true,
            list_of(containing_statements, 2, 
                new_expression_statement(new_identifier_expression("d", NULL)), 
                new_expression_statement(new_identifier_expression("e", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("while (a) b;", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 1, new_expression_statement(new_identifier_expression("b", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("while (a) { b; c; }", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 2, 
                new_expression_statement(new_identifier_expression("b", NULL)), 
                new_expression_statement(new_identifier_expression("c", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("while (a) { break; continue; }", false, 
        new_while_statement(
            new_identifier_expression("a", NULL),
            list_of(containing_statements, 2, 
                new_break_statement(), 
                new_continue_statement())
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("for(a; b; c) d;", false, 
        new_for_statement(
            new_identifier_expression("a", NULL),
            new_identifier_expression("b", NULL),
            new_identifier_expression("c", NULL),
            list_of(containing_statements, 1, new_expression_statement(new_identifier_expression("d", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("for(a; b; c) { d; e; }", false, 
        new_for_statement(
            new_identifier_expression("a", NULL),
            new_identifier_expression("b", NULL),
            new_identifier_expression("c", NULL),
            list_of(containing_statements, 2, 
                new_expression_statement(new_identifier_expression("d", NULL)),
                new_expression_statement(new_identifier_expression("e", NULL)))
        ), 
    verbose)) all_passed = false;

    if (!use_case_passes("for (a) { c; d; }", true, NULL, verbose)) all_passed = false;
    if (!use_case_passes("for (a;b) { c; d; }", true, NULL, verbose)) all_passed = false;
    // if (!use_case_passes("for (a;b;c;d) { e; f; }", true, NULL, verbose)) all_passed = false;

    return all_passed;
}
