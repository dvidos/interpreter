#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "debugger.h"
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/data_types/exec_context.h"

bool debugger_session_done;


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

static void show_curr_code_line(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    int curr_line_no = (curr_stmt != NULL ? curr_stmt->token->line_no : (curr_expr != NULL ? curr_expr->token->line_no : 0));
    fprintf(stdout, "%3d %c  %c   %s\n",
        curr_line_no, 
        ' ', // 'B' for break point
        '>',
        listing_get_line(ctx->code_listing, curr_line_no)
    );
}
static void list_code(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    int context_lines = 5;
    int curr_line_no = (curr_stmt != NULL ? curr_stmt->token->line_no : (curr_expr != NULL ? curr_expr->token->line_no : 0));
    int min_line_no = curr_line_no - context_lines;
    int max_line_no = curr_line_no + context_lines;
    int total_lines = listing_lines_count(ctx->code_listing);
    if (min_line_no > total_lines) min_line_no = total_lines - context_lines;
    if (min_line_no < 1) min_line_no = 1;
    if (max_line_no >= total_lines) max_line_no = total_lines - 1;

    for (int line_no = min_line_no; line_no <= max_line_no; line_no++) {
        fprintf(stdout, "%3d %c  %c   %s\n",
            line_no, 
            ' ', // 'B' for break point
            line_no == curr_line_no ? '>' : ' ',
            listing_get_line(ctx->code_listing, line_no)
        );
    }
}

static failable handle_command(char *cmd, statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    if (strncmp(cmd, "h", 1) == 0) {
        show_help();
    } else if (strncmp(cmd, "l", 1) == 0) {
        list_code(curr_stmt, curr_expr, ctx);
    } else if (strncmp(cmd, "c", 1) == 0) {
        debugger_session_done = true;
    } else if (strncmp(cmd, "q", 1) == 0) {
        return failed(NULL, "Execution aborted!");
    }

    return ok();
}

static bool get_command(char *buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    return true;
}

bool should_start_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    if (!ctx->debugger_enabled)
        return false;
    if (curr_stmt != NULL) {
        if (curr_stmt->type == ST_BREAKPOINT)
            return true;
        if (curr_stmt->type == ST_RETURN &&
            ctx->debugger_flags.break_mode == DBM_NEXT_RETURN_STATEMENT)
            return true;
    }
    if (curr_expr != NULL) {
        // ???
    }
    return false;
}

failable run_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    char buffer[128];

    printf("Inline debugger. h for help.\n");
    show_curr_code_line(curr_stmt, curr_expr, ctx);
    debugger_session_done = false;

    while (!debugger_session_done) {
        // should print current line.
        fputs("dbg > ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        
        failable handling = handle_command(buffer, curr_stmt, curr_expr, ctx);
        if (handling.failed) return failed(&handling, NULL);
    }
}
