#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "debugger.h"
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/data_types/exec_context.h"

static void show_help() {
    printf("  n -- next (run till next line number)\n");
    printf("  s -- step (run till next statement or expression)\n");
    printf("  r -- return (run till next return statement)\n");
    printf("  c -- continue execution\n");
    printf("  q -- quit execution\n");
    printf("\n");
    printf("  l -- list code\n");
    printf("  a -- print function args and current values\n");
    printf("  w -- where (print stack trace)\n");
    printf("\n");
    printf("  b func_name | [file:] line_no -- add breakpoint to function or file:line\n");
    printf("  p expresion -- evaluate and print expression\n");
}

static void handle_command(char *cmd) {
    if (strncmp(cmd, "h", 1) == 0) {
        show_help();
    }
}

static bool get_command(char *buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    return true;
}

bool should_start_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    return false;
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
