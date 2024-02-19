#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "utils/testing.h"
#include "utils/file.h"
#include "utils/containers/_module.h"
#include "utils/data_types/_module.h"
#include "lexer/_module.h"
#include "parser/expression_parser_tests.h"
#include "parser/statement_parser_tests.h"
#include "interpreter/interpreter_tests.h"
#include "interpreter/interpreter.h"
#include "runtime/_module.h"

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
    testing_initialize();

    variant_self_diagnostics(verbose);
    containers_self_diagnostics(verbose);
    lexer_self_diagnostics(verbose);
    expression_parser_self_diagnostics(verbose);
    statement_parser_self_diagnostics(verbose);
    interpreter_self_diagnostics(verbose);
    built_in_self_diagnostics(verbose);
    
    return testing_outcome();
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
    bool suppress_log_echo;
    bool log_to_file;
    char *log_filename;
    bool enable_debugger;
    bool start_interactive_shell;
} options;

void parse_options(int argc, char *argv[]) {
    memset(&options, 0, sizeof(options));

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'e':
                    options.execute_expression = true;
                    options.expression = argv[++i];
                    break;
                case 'f':
                    options.execute_script = true;
                    options.script_filename = argv[++i];
                    break;
                case 'l':
                    options.log_to_file = true;
                    options.log_filename = argv[++i];
                    break;
                case 'h': options.show_help = true; break;
                case 'v': options.verbose = true; break;
                case 'b': options.show_built_in_functions = true; break;
                case 'u': options.run_unit_tests = true; break;
                case 'q': options.suppress_log_echo = true; break;
                case 'd': options.enable_debugger = true; break;
                case 'i': options.start_interactive_shell = true; break;
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
    printf("  -i                  Start interactive shell");
    printf("  -d                  Enable debugger\n");
    printf("  -b                  Show built in functions\n");
    printf("  -v                  Be verbose\n");
    printf("  -u                  Run self diagnostics (unit tests)\n");
    printf("  -h                  Show this help message\n");
    printf("  -q                  Suppress log() output to stderr\n");
    printf("  -l <log-file>       Save log() output to file\n");
}

void execute_code(const char *code, const char *filename) {
    printf("Executing %s...\n", filename);
    dict *values = new_dict(variant_class);
    failable_variant execution = interpret_and_execute(code, filename, values, options.verbose, options.enable_debugger);
    if (execution.failed)
        failable_print(&execution);
    else {
        str_builder *sb = new_str_builder();
        variant_describe(execution.result, sb);
        printf("Execution was successful, result is %s\n", str_builder_charptr(sb));
        str_builder_free(sb);
    }
}

void execute_script(const char *filename) {
    failable_const_char contents = file_read(filename);
    if (contents.failed) {
        printf("%s", contents.err_msg);
        return;
    }
    execute_code(contents.result, filename);
}

void execute_shell() {
    void interactive_shell(bool verbose, bool enable_debugger);
    interactive_shell(options.verbose, options.enable_debugger);
}


FILE *log_file = NULL;

void setup() {

    initialize_interpreter();
    if (options.log_to_file)
        exec_context_set_log_echo(NULL, options.log_filename);
    else if (!options.suppress_log_echo)
        exec_context_set_log_echo(stderr, NULL);
}

int main(int argc, char *argv[]) {
    
    parse_options(argc, argv);
    setup();

    if (options.show_help) {
        show_help();
    } else if (options.run_unit_tests) {
        if (!run_self_diagnostics(options.verbose))
            return 1;
    } else if (options.show_built_in_functions) {
        printf("Built-in functions:\n");
        print_built_in_funcs_list();
    } else if (options.execute_expression) {
        execute_code(options.expression, "inline");
    } else if (options.execute_script) {
        execute_script(options.script_filename);
    } else if (options.show_help) {
        execute_script(options.script_filename);
    } else if (options.start_interactive_shell) {
        execute_shell();
    } else {
        show_help();
    }

    return 0;
}
