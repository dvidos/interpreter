#ifndef _EXEC_CONTEXT_H
#define _EXEC_CONTEXT_H

#include <stdio.h>
#include <stdbool.h>
#include "../../containers/_module.h"
#include "stack_frame.h"
#include "../../utils/listing.h"

typedef struct exec_context exec_context;
struct exec_context {
    bool verbose;

    const char *script_name;
    listing *code_listing;
    list *ast_root_statements;
    dict *built_in_symbols;
    dict *global_values;
    dict *constructable_variant_types;
    stack *stack_frames;

    struct debugger_info {
        bool enabled;
        bool enter_at_next_instruction;
        bool enter_when_at_different_line;
        const char *original_filename;
        int original_line_no;
        bool enter_at_next_return;
        int return_stack_size;
        list *breakpoints;
    } debugger;

    // stdin, stdout, logger
};

exec_context *new_exec_context(const char *script_name, listing *code_listing, list *ast_root_statments, dict *global_values, bool verbose, bool debugger, bool start_with_debugger);

stack_frame *exec_context_get_curr_stack_frame(exec_context *c);
failable exec_context_push_stack_frame(exec_context *c, stack_frame *f);
failable exec_context_pop_stack_frame(exec_context *c);

failable exec_context_register_built_in(exec_context *c, const char *name, variant *value);

variant *exec_context_resolve_symbol(exec_context *c, const char *name);
bool exec_context_symbol_exists(exec_context *c, const char *name);
failable exec_context_register_symbol(exec_context *c, const char *name, variant *v);
failable exec_context_update_symbol(exec_context *c, const char *name, variant *v);
failable exec_context_unregister_symbol(exec_context *c, const char *name);
bool exec_context_is_curr_method_owned_by(exec_context *c, variant_type *class_type);

failable exec_context_register_constructable_type(exec_context *c, variant_type *type);
variant_type *exec_context_get_constructable_type(exec_context *c, const char *name);



// simple things for now.
void        exec_context_log_reset();
void        exec_context_log_line(const char *line);
void        exec_context_log_str(const char *str);
const char *exec_context_get_log();

FILE *exec_context_get_log_echo();
void exec_context_set_log_echo(FILE *handle, char *filename);


#endif
