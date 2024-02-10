#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "debugger.h"
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/data_types/exec_context.h"
#include "../utils/str.h"
#include "../utils/str_builder.h"

bool debugger_session_done;
#define between(num, min, max)  ((num)<(min)?(min):((num)>(max)?(max):(num)))


static void show_help() {
    printf("  n -- next (run till next line number)\n");
    printf("  s -- step (run till next statement or expression)\n");
    printf("  r -- return (run till next return statement)\n");
    printf("  c -- continue execution\n");
    printf("  q -- quit execution\n");
    printf("\n");
    printf("  l -- list code, l <line_no> [, <lines_to_show> ]\n");
    printf("  a -- print function args and current values\n");
    printf("  w -- where (print stack trace)\n");
    printf("\n");
    printf("  b func_name | [file:] line_no -- add breakpoint to function or file:line\n");
    printf("  p expresion -- evaluate and print expression\n");
}

static void show_curr_code_line(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    int curr_line_no = (curr_stmt != NULL ? curr_stmt->token->line_no : (curr_expr != NULL ? curr_expr->token->line_no : 0));
    fprintf(stdout, "%3d %c  %s   %s\n",
        curr_line_no, 
        ' ', // 'B' for break point
        "-->",
        listing_get_line(ctx->code_listing, curr_line_no)
    );
}

static void list_code(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *line_no_arg) {
    int curr_line_no = (curr_stmt != NULL ? curr_stmt->token->line_no : (curr_expr != NULL ? curr_expr->token->line_no : 0));
    int lines_count = listing_lines_count(ctx->code_listing);
    
    int context_lines = 5;
    int line_to_show = (strlen(line_no_arg) > 0) ? atoi(line_no_arg) : curr_line_no;
    line_to_show = between(line_to_show, 1, lines_count);
    int min_line_to_show = between(line_to_show - context_lines, 1, lines_count);
    int max_line_to_show = between(line_to_show + context_lines, 1, lines_count);

    for (int n = min_line_to_show; n <= max_line_to_show; n++) {
        fprintf(stdout, "%3d %c  %s   %s\n",
            n, 
            ' ', // 'B' for break point
            n == curr_line_no ? "-->" : "   ",
            listing_get_line(ctx->code_listing, n)
        );
    }
}

static void show_stack_trace(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    int level = stack_length(ctx->stack_frames);

    for_stack(ctx->stack_frames, sfit, stack_frame, f) {
        token *t = f->func_stmt == NULL ? (f->func_expr == NULL ? NULL : f->func_expr->token) : f->func_stmt->token;
        if (f->func_stmt != NULL) t = f->func_stmt->token;
        if (f->func_expr != NULL) t = f->func_expr->token;
        printf("   %2d   %s(), at %s:%d:%d\n", 
            level--,
            f->func_name,
            t == NULL ? "(unknown)" : t->filename,
            t == NULL ? 0 : t->line_no,
            t == NULL ? 0 : t->column_no
        );
    }
    printf("   %2d   %s\n", 0, ctx->script_name);
}

static void show_values_of_symbols_of(dict *symbols) {
    str_builder *sb = new_str_builder();
    for_dict(symbols, sit, str, name) {
        str_builder_clear(sb);
        variant *v = dict_get(symbols, name);
        if (v == NULL)
            str_builder_add(sb, "(null)");
        else 
            variant_describe(v, sb);
        printf("   %s: %s\n", name, str_builder_charptr(sb));
    }
    str_builder_free(sb);
}

static void show_args_and_values(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    stack_frame *f = stack_peek(ctx->stack_frames);
    if (f == NULL) {
        // show globals
        show_values_of_symbols_of(ctx->global_symbols);
    } else {
        // show locals
        show_values_of_symbols_of(f->symbols);
    }
}

static failable handle_command(char *cmd, statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    if      (strncmp(cmd, "h", 1) == 0) show_help();
    else if (strncmp(cmd, "l", 1) == 0) list_code(curr_stmt, curr_expr, ctx, cmd + 1);
    else if (strncmp(cmd, "c", 1) == 0) debugger_session_done = true;
    else if (strncmp(cmd, "q", 1) == 0) return failed(NULL, "Execution aborted!");
    else if (strncmp(cmd, "w", 1) == 0) show_stack_trace(curr_stmt, curr_expr, ctx);
    else if (strncmp(cmd, "a", 1) == 0) show_args_and_values(curr_stmt, curr_expr, ctx);

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

    printf("Inline debugger. Enter 'h' for help.\n");
    show_curr_code_line(curr_stmt, curr_expr, ctx);
    debugger_session_done = false;

    while (!debugger_session_done) {
        fputs("debug: ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        
        failable handling = handle_command(buffer, curr_stmt, curr_expr, ctx);
        if (handling.failed) return failed(&handling, NULL);
    }
    return ok();
}
