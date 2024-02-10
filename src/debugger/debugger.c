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

#define between(num, min, max)        ((num)<(min)?(min):((num)>(max)?(max):(num)))
#define get_curr_filename(stmt, expr) ((stmt) != NULL ? (stmt)->token->filename : ( \
                                       (expr) != NULL ? (expr)->token->filename : NULL))
#define get_curr_line_no(stmt, expr)  ((stmt) != NULL ? (stmt)->token->line_no : ( \
                                       (expr) != NULL ? (expr)->token->line_no : 0))



static void show_help() {
    printf("  n -- next (run till next line number)\n");
    printf("  s -- step (run till next statement or expression)\n");
    printf("  r -- return (run till next return statement)\n");
    printf("  c -- continue execution\n");
    printf("  q -- quit execution\n");
    printf("\n");
    printf("  l -- list code, l [+-]<line_no> [, <lines_to_show> ]\n");
    printf("  a -- print function args and current values\n");
    printf("  w -- where (print stack trace)\n");
    printf("\n");
    printf("  b [ func_name | [file:]line_no ] -- toggle breakpoint to function or file:line\n");
    printf("  p expresion -- evaluate and print expression\n");
}

static void show_curr_code_line(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    int line_no = get_curr_line_no(curr_stmt, curr_expr);
    fprintf(stdout, "%3d %c  %s   %s\n",
        line_no, 
        ' ', // 'B' for break point
        "-->",
        listing_get_line(ctx->code_listing, line_no)
    );
}

static void list_code(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {
    int line_no = get_curr_line_no(curr_stmt, curr_expr);
    int lines_count = listing_lines_count(ctx->code_listing);
    
    int context_lines = 8;
    int line_to_show = (strlen(cmd_arg) > 0) ? atoi(cmd_arg) : line_no;
    line_to_show = between(line_to_show, 1, lines_count);
    int min_line_to_show = between(line_to_show - context_lines, 1, lines_count);
    int max_line_to_show = between(line_to_show + context_lines, 1, lines_count);

    for (int n = min_line_to_show; n <= max_line_to_show; n++) {
        fprintf(stdout, "%3d %c  %s   %s\n",
            n, 
            ' ', // 'B' for break point
            n == line_no ? "-->" : "   ",
            listing_get_line(ctx->code_listing, n)
        );
    }
}

static void manipulate_breakpoint(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {
    int line_no = get_curr_line_no(curr_stmt, curr_expr);
    int break_line;
    if (strlen(cmd_arg) > 0) {
        break_line = between(atoi(cmd_arg), 1, listing_lines_count(ctx->code_listing));
    } else {
        break_line = line_no;
    }
    printf("Will toggle breakpoint at line %d\n", break_line);
    // should insert breakpoint in AST, in the first mentioning of this line.
    // interface:
    // - set_breakpoint, get_breakpoint, del_breakpoint, list_breakpoints???
    
}

static void print_expression(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {

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
        else if (variant_is_callable(v))
            str_builder_add(sb, "(callable)");
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

static void setup_stepping(bool single, bool next, bool cont, statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    if (single) {
        ctx->debugger.enter_at_next_instruction = true;
        ctx->debugger.enter_when_at_different_line = true;
        ctx->debugger.original_filename = NULL;
        ctx->debugger.original_line_no = 0;
    } else if (next) {
        ctx->debugger.enter_at_next_instruction = false;
        ctx->debugger.enter_when_at_different_line = true;
        ctx->debugger.original_filename = get_curr_filename(curr_stmt, curr_expr);
        ctx->debugger.original_line_no = get_curr_line_no(curr_stmt, curr_expr);
    } else if (cont) {
        ctx->debugger.enter_at_next_instruction = false;
        ctx->debugger.enter_when_at_different_line = false;
        ctx->debugger.original_filename = NULL;
        ctx->debugger.original_line_no = 0;
    }
}

static failable_bool handle_command(char *cmd, statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    bool done = false;
    switch (cmd[0]) {
        case 'h': show_help(); break;
        case 'l': list_code(curr_stmt, curr_expr, ctx, cmd + 1); break;
        case 's': setup_stepping(true, false, false, curr_stmt, curr_expr, ctx); done = true; break;
        case 'n': setup_stepping(false, true, false, curr_stmt, curr_expr, ctx); done = true; break;
        case 'c': setup_stepping(false, false, true, curr_stmt, curr_expr, ctx); done = true; break;
        case 'q': return failed_bool(NULL, "Execution aborted!"); break;
        case 'w': show_stack_trace(curr_stmt, curr_expr, ctx); break;
        case 'a': show_args_and_values(curr_stmt, curr_expr, ctx); break;
        case 'b': manipulate_breakpoint(curr_stmt, curr_expr, ctx, cmd + 1); break;
        case 'p': print_expression(curr_stmt, curr_expr, ctx, cmd + 1); break;
        default : printf("unknown command, enter 'h' for help\n"); break;
    }
    return ok_bool(done);
}

static bool get_command(char *buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    
    return true;
}

bool should_start_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    if (!ctx->debugger.enabled)
        return false;
    
    if (ctx->debugger.enter_at_next_instruction)
        return true;
    
    if (ctx->debugger.enter_when_at_different_line) {
        const char *curr_filename = get_curr_filename(curr_stmt, curr_expr);
        if (strcmp(curr_filename, ctx->debugger.original_filename) != 0)
            return true;
        int curr_line_no = get_curr_line_no(curr_stmt, curr_expr);
        return (curr_line_no != ctx->debugger.original_line_no);
    }

    if (curr_stmt != NULL && curr_stmt->type == ST_BREAKPOINT)
        return true;
    
    return false;
}

failable run_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    char buffer[128];
    bool done = false;

    printf("Inline debugger. Enter 'h' for help.\n");
    show_curr_code_line(curr_stmt, curr_expr, ctx);

    while (!done) {
        fputs("debug: ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        
        failable_bool handling = handle_command(buffer, curr_stmt, curr_expr, ctx);
        if (handling.failed) return failed(&handling, NULL);
        done = handling.result;
    }
    return ok();
}
