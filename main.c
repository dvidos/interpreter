#include "utils/testing.h"
#include "utils/variant.h"
#include "utils/file.h"
#include "interpreter/lexer/tokenization_tests.h"
#include "interpreter/parser/expression_parser_tests.h"
#include "interpreter/interpreter_tests.h"
#include "interpreter/interpreter.h"
#include "interpreter/runtime/built_in_funcs.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
    The core of the functionality is the "interpret_and_execute()" function.
    It would give the ability to programs to evaluate strings,
    like complex business conditions etc.

    To do this we need a whole ecosystem for these:
    - values (strings, numbers, booleans, lists, dictionaries)
    - parser of expressions with precedence, into an abstract syntax tree
    - evaluation of the syntax tree, to find the result

    We would love to include function calls in the expression parser,
    something like "if(left(a, 1) == '0', 'number', 'letter')"
    Maybe the code body, arguments and returned value can become a function.

    interpret_and_execute("a + b * 2", {a=1, b=2})
*/

bool run_self_diagnostics(bool verbose) {

    variant_self_diagnostics();
    tokenizer_self_diagnostics();
    expression_parser_self_diagnostics(false);
    interpreter_self_diagnostics();

    return testing_outcome(verbose);
}

struct options {
    bool verbose;
    bool show_help;
    bool run_unit_tests;
    bool show_built_in_functions;
    bool execute_expression;
    char *expression;
    bool execute_script;
    char *script_filename;
} options;

void parse_options(int argc, char *argv[]) {
    memset(&options, 0, sizeof(options));

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h': options.show_help = true; break;
                case 'v': options.verbose = true; break;
                case 'b': options.show_built_in_functions = true; break;
                case 'e':
                    options.execute_expression = true;
                    options.expression = argv[++i];
                    break;
                case 'f':
                    options.execute_script = true;
                    options.script_filename = argv[++i];
                    break;
                case 'u':
                    options.run_unit_tests = true;
                    break;
            }
        }
    }
}

void show_help() {
    printf("A small C-like interpreter, an exercize to learn\n");
    printf("Usage: ipret <options>\n");
    printf("Options:\n");
    printf("  -f <script-file>    Load and interpret a script file\n");
    printf("  -e <expression>     Interpret and execute the expression\n");
    printf("  -b                  Show built in functions\n");
    printf("  -v                  Be verbose\n");
    printf("  -u                  Run self diagnostics (unit tests)\n");
    printf("  -h                  Show this help message\n");
}

void execute_code(const char *code) {
    dict *values = new_dict(containing_variants, 20);
    failable_variant execution = interpret_and_execute(code, values, options.verbose);
    if (execution.failed)
        printf("Failed: %s\n", execution.err_msg);
    else
        printf("Result: %s\n", variant_to_string(execution.result));
}

void execute_script(const char *filename) {
    failable_constcharptr contents = file_read(filename);
    if (contents.failed) {
        printf("%s", contents.err_msg);
        return;
    }
    execute_code(contents.result);
}

int main(int argc, char *argv[]) {
    initialize_interpreter();

    parse_options(argc, argv);

    if (options.show_help) {
        show_help();
    } else if (options.run_unit_tests) {
        if (!run_self_diagnostics(true))
            return 1;
    } else if (options.show_built_in_functions) {
        printf("Built-in functions:\n");
        print_built_in_funcs_list();
    } else if (options.execute_expression) {
        execute_code(options.expression);
    } else if (options.execute_script) {
        execute_script(options.script_filename);
    } else {
        show_help();
    }

    return 0;
}
