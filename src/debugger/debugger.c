#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "debugger.h"
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/data_types/exec_context.h"

/*
    h - help
    n - run till next line
    s - step into
    b [func_name] | [[file :] line_no] - add breakpoint to file:line
    r = run till "return"
    c - continue execution (till next breakpoint)
    q - quit execution
    l - list code
    a - print function args and current values
    p expr - evaluate and print expression
    w = list stack frames, also "bt" for backtrace
*/




void handle_command(char *cmd) {
}

bool get_command(char *buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    return true;
}

void run_debugger(statement *curr_statement, expression *curr_expression, exec_context *ctx) {
    char buffer[128];
    while (true) {
        fputs("dbg (h=help) > ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        handle_command(buffer);
    }
}