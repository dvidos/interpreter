#ifndef _MEM_H
#define _MEM_H

#include <stdlib.h>

#define malloc(size)        __mem_alloc(size, #size, __FILE__, __LINE__)
#define realloc(ptr, size)  __mem_realloc(ptr, size, #size, __FILE__, __LINE__)
#define free(ptr)           __mem_free(ptr, __FILE__, __LINE__)

void *__mem_alloc(int size, const char *size_str, const char *file, int line);
void *__mem_realloc(void *ptr, int new_size, const char *size_str, const char *file, int line);
void __mem_free(void *ptr, const char *file, int line);

long mem_stats_bytes_allocated();
long mem_stats_bytes_freed();
long mem_stats_bytes_housekeeping();
long mem_stats_allocations();
long mem_stats_freeings();
void mem_set_verbose_mode(int verbose);


void mem_stats_take_snapshot();
void mem_stats_compare_snapshot();


#endif
