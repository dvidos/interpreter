#ifndef _EXPERIMENTS_H
#define _EXPERIMENTS_H






 
typedef struct logger logger;
void log_info(logger *l, const char *fmt, ...);
void log_warn(logger *l, const char *fmt, ...);
void log_error(logger *l, const char *fmt, ...);





#endif
