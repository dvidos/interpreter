#ifndef _EXEC_CONTEXT_H
#define _EXEC_CONTEXT_H



// simple things for now.
void        exec_context_log_reset();
void        exec_context_log_line(const char *line);
void        exec_context_log_str(const char *str);
const char *exec_context_get_log();




#endif
