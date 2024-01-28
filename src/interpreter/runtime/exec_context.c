#include <stdlib.h>
#include <stdbool.h>
#include "../../utils/str_builder.h"
#include "exec_context.h"
#include "built_in_funcs.h"
#include "symbol_table.h"


exec_context *new_exec_context(bool verbose) {
    exec_context *c = malloc(sizeof(exec_context));
    c->verbose = verbose;
    c->symbols = new_symbol_table(NULL);
    return c;
}




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
