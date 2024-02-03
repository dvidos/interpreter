#include <stdlib.h>
#include <stdbool.h>
#include "../utils/str_builder.h"
#include "exec_context.h"
#include "built_in_funcs.h"
#include "stack_frame.h"


exec_context *new_exec_context(bool verbose) {
    exec_context *c = malloc(sizeof(exec_context));
    c->verbose = verbose;
    c->global_symbols = new_dict(containing_variants, 32);
    c->stack_frames = new_stack(containing_stack_frames);
    return c;
}

stack_frame *exec_context_get_curr_stack_frame(exec_context *c) {
    return stack_empty(c->stack_frames) ? 
        NULL : stack_peek(c->stack_frames);
}

failable exec_context_push_stack_frame(exec_context *c, stack_frame *f) {
    stack_push(c->stack_frames, f);
    return ok();
}

failable exec_context_pop_stack_frame(exec_context *c) {
    if (stack_empty(c->stack_frames))
        return failed(NULL, "Cannot pop stack frame, stack already empty");
    stack_pop(c->stack_frames);
    return ok();
}

variant *exec_context_resolve_symbol(exec_context *c, const char *name) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        variant *v = stack_frame_resolve_symbol(f, name);
        if (v != NULL)
            return v;
    }

    if (dict_has(c->global_symbols, name))
        return dict_get(c->global_symbols, name);
    return NULL;
}

bool exec_context_symbol_exists(exec_context *c, const char *name) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        if (stack_frame_symbol_exists(f, name))
            return true;
    }

    return dict_has(c->global_symbols, name);
}

failable exec_context_register_symbol(exec_context *c, const char *name, variant *v) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        failable registration = stack_frame_register_symbol(f, name, v);
        if (registration.failed) return failed(&registration, NULL);
        return ok();
    }
    
    if (dict_has(c->global_symbols, name))
        return failed("Symbol %s already exists", name);
    dict_set(c->global_symbols, name, v);
    return ok();
}

failable exec_context_update_symbol(exec_context *c, const char *name, variant *v) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        if (stack_frame_symbol_exists(f, name))
            return stack_frame_update_symbol(f, name, v);
    }
    
    if (!dict_has(c->global_symbols, name))
        return failed("Symbol %s does not exist", name);
    dict_set(c->global_symbols, name, v);
    return ok();
}





// ---------------------------

static str_builder *exec_context_log = NULL;

void exec_context_log_reset() {
    if (exec_context_log == NULL)
        exec_context_log = new_str_builder();
    
    str_builder_clear(exec_context_log);
}

void exec_context_log_line(const char *line) {
    if (exec_context_log == NULL)
        exec_context_log = new_str_builder();

    str_builder_cat(exec_context_log, line);
    str_builder_cat(exec_context_log, "\n");
}

void exec_context_log_str(const char *str) {
    if (exec_context_log == NULL)
        exec_context_log = new_str_builder();

    str_builder_cat(exec_context_log, str);
}

const char *exec_context_get_log() {
    if (exec_context_log == NULL)
        exec_context_log = new_str_builder();

    return str_builder_charptr(exec_context_log);
}

static FILE *log_echo_handle = NULL;

FILE *exec_context_get_log_echo() {
    return log_echo_handle;
}

void exec_context_set_log_echo(FILE *handle, char *filename) {
    if (handle != NULL) {
        log_echo_handle = handle;
    } else if (filename != NULL) {
        log_echo_handle = fopen(filename, "a");
    }
}
