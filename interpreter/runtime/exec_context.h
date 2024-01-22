#ifndef _EXEC_CONTEXT_H
#define _EXEC_CONTEXT_H

#include <stdio.h>
#include "../../utils/containers/_module.h"


typedef struct exec_context {
    bool verbose;
    dict *global_variables;
    dict *callables;
} exec_context;



// simple things for now.
void        exec_context_log_reset();
void        exec_context_log_line(const char *line);
void        exec_context_log_str(const char *str);
const char *exec_context_get_log();

FILE *exec_context_get_log_echo();
void exec_context_set_log_echo(FILE *handle, char *filename);


#endif
