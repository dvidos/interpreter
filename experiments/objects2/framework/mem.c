#include <stdlib.h>
#include <string.h>
#include "mem.h"

void *mem_alloc(int size) {
    // much more capabilities here, from tracking to ref counting etc.
    size = size <= 0 ? 1 : size;
    void *p = malloc(size);
    if (p == NULL) return NULL;
    memset(p, 0, size);
    return p;
}

void *mem_realloc(void *ptr, int size) {
    // much more capabilities here, from tracking to ref counting etc.
    size = size <= 0 ? 1 : size;
    return realloc(ptr, size);
}

void mem_free(void **ptr_ptr) {
    // much more capabilities here, from tracking to ref counting etc.
    if ((*ptr_ptr) != NULL)
        free(*ptr_ptr);
    *ptr_ptr = NULL;
}

