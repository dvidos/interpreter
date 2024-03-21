#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "debugger.h"
#include "breakpoint.h"
#include "../interpreter/interpreter.h"
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/cstr.h"
#include "../utils/str.h"

#define between(num, min, max)        ((num)<(min)?(min):((num)>(max)?(max):(num)))
#define get_curr_filename(stmt, expr) ((stmt) != NULL ? (stmt)->token->origin->filename : ( \
                                       (expr) != NULL ? (expr)->token->origin->filename : NULL))
#define get_curr_line_no(stmt, expr)  ((stmt) != NULL ? (stmt)->token->origin->line_no : ( \
                                       (expr) != NULL ? (expr)->token->origin->line_no : 0))


enum ast_task { AST_ADD_BREAKPOINT, AST_DEL_BREAKPOINT };
static bool walk_ast_statements(list *statements, enum ast_task task, const char *filename, int line_no);
static bool walk_ast_statement(statement *statement, enum ast_task task, const char *filename, int line_no);


static bool walk_ast_statements(list *statements, enum ast_task task, const char *filename, int line_no) {
    if (statements == NULL)
        return false;
    
    if (task == AST_ADD_BREAKPOINT) {
        int index = 0;
        for_list(statements, it, statement, stmt) {
            if (statement_is_at(stmt, filename, line_no)) {
                list_insert(statements, index, new_breakpoint_statement(stmt->token));
                return true;
            }
            index++;
        }
    } else if (task == AST_DEL_BREAKPOINT) {
        int index = 0;
        for_list(statements, it, statement, stmt) {
            if (stmt->type == ST_BREAKPOINT && statement_is_at(stmt, filename, line_no)) {
                list_remove(statements, index);
                return true;
            }
        }
        index++;
    } else {
        return false;
    }

    // if we got here, it means we did not find target, go deeper
    for_list(statements, it, statement, stmt) {
        if (walk_ast_statement(stmt, task, filename, line_no))
            return true;
    }

    return false;
}

static bool walk_ast_statement(statement *stmt, enum ast_task task, const char *filename, int line_no) {
    bool done;
    if (stmt->type == ST_IF) {
        done = walk_ast_statements(stmt->per_type.if_.body_statements, task, filename, line_no);
        if (done) return true;
        if (stmt->per_type.if_.has_else) {
            done = walk_ast_statements(stmt->per_type.if_.else_body_statements, task, filename, line_no);
            if (done) return true;
        }
    } else if (stmt->type == ST_WHILE) {
        done = walk_ast_statements(stmt->per_type.while_.body_statements, task, filename, line_no);
        if (done) return true;
    } else if (stmt->type == ST_FOR_LOOP) {
        done = walk_ast_statements(stmt->per_type.for_.body_statements, task, filename, line_no);
        if (done) return true;
    } else if (stmt->type == ST_FUNCTION) {
        done = walk_ast_statements(stmt->per_type.function.statements, task, filename, line_no);
        if (done) return true;
    } else if (stmt->type == ST_TRY_CATCH) {
        done = walk_ast_statements(stmt->per_type.try_catch.try_statements, task, filename, line_no);
        if (done) return true;
        done = walk_ast_statements(stmt->per_type.try_catch.catch_statements, task, filename, line_no);
        if (done) return true;
        done = walk_ast_statements(stmt->per_type.try_catch.finally_statements, task, filename, line_no);
        if (done) return true;
    } else if (stmt->type == ST_THROW) {
        // nothing
    }
    return false;
}

static void list_breakpoints(exec_context *ctx) {
    if (list_empty(ctx->debugger.breakpoints)) {
        printf("  no breakpoints\n");
        return;
    }
    for_list(ctx->debugger.breakpoints, it, breakpoint, b) {
        printf("  %s : %d\n", b->filename, b->line_no);
    }
}

static bool is_breakpoint_at_line(const char *filename, int line_no, exec_context *ctx) {
    for_list(ctx->debugger.breakpoints, it, breakpoint, bp) {
        if (breakpoint_is_at(bp, filename, line_no))
            return true;
    }
    return false;
}

static void insert_breakpoint_at_line(const char *filename, int line_no, exec_context *ctx) {
    if (!walk_ast_statements(ctx->ast_root_statements, AST_ADD_BREAKPOINT, filename, line_no))
        return;
    list_add(ctx->debugger.breakpoints, new_breakpoint(filename, line_no));
    printf("Added breakpoint at %s:%d\n", filename, line_no);
}

static void remove_breakpoint_at_line(const char *filename, int line_no, exec_context *ctx) {
    int index = 0;
    bool found = false;
    for_list(ctx->debugger.breakpoints, it, breakpoint, bp) {
        if (breakpoint_is_at(bp, filename, line_no)) {
            found = true;
            list_remove(ctx->debugger.breakpoints, index);
        }
        index++;
    }
    if (!found)
        return;
    
    walk_ast_statements(ctx->ast_root_statements, AST_DEL_BREAKPOINT, filename, line_no);
    printf("Removed breakpoint from %s:%d\n", filename, line_no);
}

static void show_help() {
    printf("  n -- next (run till next line number)\n");
    printf("  s -- step (run till next statement or expression)\n");
    printf("  r -- return (run till next return statement)\n");
    printf("  c -- continue execution\n");
    printf("  q -- quit execution\n");
    printf("\n");
    printf("  l -- list code, l [+-]<line_no> [, <lines_to_show> ]\n");
    printf("  a -- print function args and current values\n");
    printf("  g -- print global variables\n");
    printf("  w -- where (print stack trace)\n");
    printf("\n");
    printf("  b [ func_name | [file:]line_no ] -- toggle breakpoint to function or file:line\n");
    printf("  p expresion -- evaluate and print expression\n");
}

static void show_curr_code_line(statement *curr_stmt, expression *curr_expr, exec_context *ctx, int extra_lines) {
    const char *filename = get_curr_filename(curr_stmt, curr_expr);
    int line_no = get_curr_line_no(curr_stmt, curr_expr);
    int lines_count = listing_lines_count(ctx->code_listing);
    int min_line_to_show = between(line_no - extra_lines, 1, lines_count);
    int max_line_to_show = between(line_no + extra_lines, 1, lines_count);

    for (int n = min_line_to_show; n <= max_line_to_show; n++) {
        fprintf(stdout, "%3d %c  %s   %s\n",
            n, 
            is_breakpoint_at_line(filename, n, ctx) ? 'B' : ' ',
            n == line_no ? "-->" : "   ",
            listing_get_line(ctx->code_listing, n)
        );
    }
}

static void list_code(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {
    const char *filename = get_curr_filename(curr_stmt, curr_expr);
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
            is_breakpoint_at_line(filename, n, ctx) ? 'B' : ' ',
            n == line_no ? "-->" : "   ",
            listing_get_line(ctx->code_listing, n)
        );
    }
}

static void manipulate_breakpoint(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {
    if (strlen(cmd_arg) == 0) { // list all
        list_breakpoints(ctx);
    } else { // toggle at line
        const char *filename = get_curr_filename(curr_stmt, curr_expr);
        int line_no = between(atoi(cmd_arg), 1, listing_lines_count(ctx->code_listing));

        if (is_breakpoint_at_line(filename, line_no, ctx)) {
            remove_breakpoint_at_line(filename, line_no, ctx);
        } else {
            insert_breakpoint_at_line(filename, line_no, ctx);
        }
    }
}

static void print_expression(statement *curr_stmt, expression *curr_expr, exec_context *ctx, char *cmd_arg) {
    // launch and evaluate an expression within the same local/global context...
    // i think we did not make our code re-entrant, so this may fail...

    dict *vars = (stack_empty(ctx->stack_frames) ? ctx->built_in_symbols : 
        ((stack_frame *)stack_peek(ctx->stack_frames))->symbols);
    
    execution_outcome ex = interpret_and_execute(cmd_arg, "(debugger)", 
        vars, false, false, false);
    
    if (ex.failed) {
        printf("Evaluation failed: %s\n", ex.failure_message);

    } else if (ex.excepted) {
        variant *s = variant_to_string(ex.exception_thrown);
        printf("Uncaught exception: %s\n", str_variant_as_str(s));
        variant_drop_ref(s);

    } else {
        variant *s = variant_to_string(ex.result);
        printf("%s\n", str_variant_as_str(s));
        variant_drop_ref(s);
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
            t == NULL ? "(unknown)" : t->origin->filename,
            t == NULL ? 0 : t->origin->line_no,
            t == NULL ? 0 : t->origin->column_no
        );
    }
    printf("   %2d   %s\n", 0, ctx->script_name);
}

static void show_values_of_symbols_of(dict *symbols) {
    str *s = new_str();
    for_dict(symbols, sit, cstr, name) {
        str_clear(s);
        variant *v = dict_get(symbols, name);
        if (v == NULL)
            str_add(s, "(null)");
        else if (variant_instance_of(v, callable_type))
            str_add(s, "(callable)");
        else {
            variant *to_string = variant_to_string(v);
            str_add(s, str_variant_as_str(to_string));
            variant_drop_ref(to_string);
        }
        printf("   %s: %s\n", name, str_cstr(s));
    }
    str_free(s);
}

static void show_args_and_values(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    stack_frame *f = stack_peek(ctx->stack_frames);
    if (f == NULL) {
        printf("Not in a function\n");
        return;
    }
    
    show_values_of_symbols_of(f->symbols);
}

static void show_globals(exec_context *ctx) {
    show_values_of_symbols_of(ctx->built_in_symbols);
}

enum step_type {
    STEP_SINGLE,
    STEP_TO_DIFF_LINE,
    STEP_TILL_RETURN,
    STEP_CONTINUE,
};

static void setup_stepping(enum step_type type, statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    // clear all flags first
    ctx->debugger.enter_at_next_instruction = false;
    ctx->debugger.enter_when_at_different_line = false;
    ctx->debugger.original_filename = NULL;
    ctx->debugger.original_line_no = 0;
    ctx->debugger.enter_at_next_return = false;
    ctx->debugger.return_stack_size = 0;

    if (type == STEP_SINGLE) {
        ctx->debugger.enter_at_next_instruction = true;
    } else if (type == STEP_TO_DIFF_LINE) {
        ctx->debugger.enter_when_at_different_line = true;
        ctx->debugger.original_filename = get_curr_filename(curr_stmt, curr_expr);
        ctx->debugger.original_line_no = get_curr_line_no(curr_stmt, curr_expr);
    } else if (type == STEP_TILL_RETURN) {
        ctx->debugger.enter_at_next_return = true;
        ctx->debugger.return_stack_size = stack_length(ctx->stack_frames);
    } else if (type == STEP_CONTINUE) {
        // nothing to change.
    }
}

static failable handle_command(char *cmd, statement *curr_stmt, expression *curr_expr, exec_context *ctx, bool *should_resume, bool *should_quit) {
    *should_resume = false;
    *should_quit = false;

    switch (cmd[0]) {
        case 'h': show_help(); break;
        case 'l': list_code(curr_stmt, curr_expr, ctx, cmd + 1); break;
        case 's': setup_stepping(STEP_SINGLE,       curr_stmt, curr_expr, ctx); *should_resume = true; break;
        case 'n': setup_stepping(STEP_TO_DIFF_LINE, curr_stmt, curr_expr, ctx); *should_resume = true; break;
        case 'r': setup_stepping(STEP_TILL_RETURN,  curr_stmt, curr_expr, ctx); *should_resume = true; break;
        case 'c': setup_stepping(STEP_CONTINUE,     curr_stmt, curr_expr, ctx); *should_resume = true; break;
        case 'q': *should_quit = true; break;
        case 'w': show_stack_trace(curr_stmt, curr_expr, ctx); break;
        case 'a': show_args_and_values(curr_stmt, curr_expr, ctx); break;
        case 'b': manipulate_breakpoint(curr_stmt, curr_expr, ctx, cmd + 1); break;
        case 'p': print_expression(curr_stmt, curr_expr, ctx, cmd + 1); break;
        case 'g': show_globals(ctx); break;
        default : printf("unknown command, enter 'h' for help\n"); break;
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

    // maybe we should also support implicit returns, i.e. after the last statement of a function.
    if (ctx->debugger.enter_at_next_return 
        && curr_stmt != NULL && curr_stmt->type == ST_RETURN
        && stack_length(ctx->stack_frames) <= ctx->debugger.return_stack_size)
        return true;

    if (curr_stmt != NULL && curr_stmt->type == ST_BREAKPOINT)
        return true;
    
    return false;
}

failable run_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx) {
    char buffer[128];

    printf("Inline debugger. Enter 'h' for help.\n");
    show_curr_code_line(curr_stmt, curr_expr, ctx, 1);

    bool should_resume;
    bool should_quit;

    while (true) {
        fputs("debug: ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        
        failable handling = handle_command(buffer, curr_stmt, curr_expr, ctx, &should_resume, &should_quit);
        if (handling.failed) return failed(&handling, NULL);

        if (should_resume)
            break; // leave the debugger loop
        if (should_quit)
            return failed(NULL, "Quit requested in debugger!");
    }

    return ok();
}
