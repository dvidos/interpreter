#include <stdlib.h>
#include <stdbool.h>
#include "../../utils/str.h"
#include "../../utils/listing.h"
#include "exec_context.h"
#include "stack_frame.h"
#include "../../debugger/breakpoint.h"



// apparently we need more than one flag for debugging,
// we need: execute single, execute whole line, execute whole function etc.
// also we need a list of breakpoints (file+line)



exec_context *new_exec_context(const char *script_name, listing *code_listing, list *ast_root_statements, dict *global_values, bool verbose, bool enable_debugger, bool start_with_debugger) {
    exec_context *c = malloc(sizeof(exec_context));
    c->script_name = script_name;
    c->code_listing = code_listing;
    c->ast_root_statements = ast_root_statements;
    c->verbose = verbose;
    c->debugger.enabled = enable_debugger;
    c->debugger.enter_at_next_instruction = start_with_debugger; // debug first line
    c->debugger.breakpoints = new_list(breakpoint_item_info);
    c->stack_frames = new_stack(stack_frame_item_info);
    c->built_in_symbols = new_dict(variant_item_info);
    c->global_values = global_values;
    c->constructable_variant_types = new_dict(NULL);
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

failable exec_context_register_built_in(exec_context *c, const char *name, variant *value) {
    if (dict_has(c->built_in_symbols, name))
        return failed("Symbol %s already exists", name);
    dict_set(c->built_in_symbols, name, value);
    return ok();
}

variant *exec_context_resolve_symbol(exec_context *c, const char *name) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        variant *v = stack_frame_resolve_symbol(f, name);
        if (v != NULL)
            return v;
    }

    if (dict_has(c->global_values, name))
        return dict_get(c->global_values, name);

    if (dict_has(c->built_in_symbols, name))
        return dict_get(c->built_in_symbols, name);
    
    if (dict_has(c->constructable_variant_types, name))
        return dict_get(c->constructable_variant_types, name);
    
    return NULL;
}

bool exec_context_symbol_exists(exec_context *c, const char *name) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        if (stack_frame_symbol_exists(f, name))
            return true;
    }

    if (dict_has(c->global_values, name))
        return true;
    
    if (dict_has(c->built_in_symbols, name))
        return true;

    if (dict_has(c->constructable_variant_types, name))
        return true;
    
    return false;
}

failable exec_context_register_symbol(exec_context *c, const char *name, variant *v) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        failable registration = stack_frame_register_symbol(f, name, v);
        if (registration.failed) return failed(&registration, NULL);
        return ok();
    }
    
    if (dict_has(c->global_values, name))
        return failed("symbol %s already exists", name);
    dict_set(c->global_values, name, v);
    return ok();
}

failable exec_context_update_symbol(exec_context *c, const char *name, variant *v) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        if (stack_frame_symbol_exists(f, name))
            return stack_frame_update_symbol(f, name, v);
    }
    
    if (!dict_has(c->global_values, name))
        return failed("Symbol %s does not exist", name);
    dict_set(c->global_values, name, v);
    return ok();
}

failable exec_context_unregister_symbol(exec_context *c, const char *name) {
    stack_frame *f = stack_peek(c->stack_frames);
    if (f != NULL) {
        failable registration = stack_frame_unregister_symbol(f, name);
        if (registration.failed) return failed(&registration, NULL);
        return ok();
    }
    
    if (!dict_has(c->global_values, name))
        return failed("symbol %s not found", name);
    dict_del(c->global_values, name);
    return ok();
}

bool exec_context_is_curr_method_owned_by(exec_context *c, variant_type *class_type) {
    if (stack_empty(c->stack_frames))
        return false;

    stack_frame *f = stack_peek(c->stack_frames);
    if (f == NULL)
        return false;

    return stack_frame_is_method_owned_by(f, class_type);
}


failable exec_context_register_constructable_type(exec_context *c, variant_type *type) {
    if (dict_has(c->constructable_variant_types, type->name))
        return failed(NULL, "type '%s' already registered", type->name);
    dict_set(c->constructable_variant_types, type->name, type);
}

variant_type *exec_context_get_constructable_type(exec_context *c, const char *name) {
    if (!dict_has(c->constructable_variant_types, name))
        return NULL;
    return (variant_type *)dict_get(c->constructable_variant_types, name);
}


// ---------------------------

static str *exec_context_log = NULL;

void exec_context_log_reset() {
    if (exec_context_log == NULL)
        exec_context_log = new_str();
    
    str_clear(exec_context_log);
}

void exec_context_log_line(const char *line) {
    if (exec_context_log == NULL)
        exec_context_log = new_str();

    str_add(exec_context_log, line);
    str_add(exec_context_log, "\n");
}

void exec_context_log_str(const char *s) {
    if (exec_context_log == NULL)
        exec_context_log = new_str();

    str_add(exec_context_log, s);
}

const char *exec_context_get_log() {
    if (exec_context_log == NULL)
        exec_context_log = new_str();

    return str_cstr(exec_context_log);
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


