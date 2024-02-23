#ifndef _MEM_H
#define _MEM_H

void *mem_alloc(int size);
void *mem_realloc(void *ptr, int size);
void mem_free(void *ptr);




#endif
