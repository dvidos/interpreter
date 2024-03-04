#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

// ensure that malloc() here is not predefined
#undef malloc
#undef realloc
#undef free

static long stats_bytes_allocated = 0;
static long stats_bytes_freed = 0;
static long stats_bytes_housekeeping = 0;
static long stats_allocations = 0;
static long stats_freeings = 0;

static long snapshot_bytes_allocated = 0;
static long snapshot_bytes_freed = 0;
static long snapshot_bytes_housekeeping = 0;
static long snapshot_allocations = 0;
static long snapshot_freeings = 0;

static char stats_verbose_mode = 0;

#define CONTROL_BLOCK_MAGIC  0x1234

// first part of block is this struct, the rest is returned to caller
typedef struct control_block {
    short magic;
    int size;
    const char *reason;
} control_block;




void *__mem_alloc(int size, const char *reason, const char *file, int line) {

    if (stats_verbose_mode)
        printf("mem: allocating %d bytes for \"%s\", at %s:%d\n", size, reason, file, line);
    
    size = size <= 0 ? 0 : size;
    void *ptr = malloc(sizeof(control_block) + size);
    if (ptr == NULL)
        return NULL;

    control_block *cb = ptr;
    ptr += sizeof(control_block);

    cb->magic = CONTROL_BLOCK_MAGIC;
    cb->size = size;
    cb->reason = reason;
    memset(ptr, 0, size);

    stats_allocations += 1;
    stats_bytes_allocated += size;
    stats_bytes_housekeeping += sizeof(control_block);

    return ptr;
}

void __mem_free(void *ptr, const char *file, int line) {
    if (ptr == NULL)
        return;
    
    control_block *cb = ptr - sizeof(control_block);
    assert(cb->magic == CONTROL_BLOCK_MAGIC);

    if (stats_verbose_mode)
        printf("mem: freeing %d bytes, at %s:%d, originally allocated for \"%s\"\n", cb->size, file, line, cb->reason);

    stats_freeings += 1;
    stats_bytes_freed += cb->size;
    stats_bytes_housekeeping -= sizeof(control_block);

    memset(cb, 0xFF, sizeof(control_block) + cb->size);
    free(cb);
}

void *__mem_realloc(void *ptr, int new_size, const char *reason, const char *file, int line) {

    if (ptr == NULL)
        return NULL;
    
    control_block *cb = ptr - sizeof(control_block);
    assert(cb->magic == CONTROL_BLOCK_MAGIC);
    if (new_size <= cb->size)
        return ptr;

    if (stats_verbose_mode)
        printf("mem: reallocating %d bytes at %s:%d, originally %d bytes, allocated for \"%s\"\n", new_size, file, line, cb->size, cb->reason);

    void *new_ptr = __mem_alloc(new_size, reason, file, line);
    if (new_ptr == NULL)
        return NULL;
    memcpy(new_ptr, ptr, cb->size);
    __mem_free(ptr, file, line);

    return new_ptr;
}

long mem_stats_bytes_allocated()    { return stats_bytes_allocated; }
long mem_stats_bytes_freed()        { return stats_bytes_freed; }
long mem_stats_bytes_housekeeping() { return stats_bytes_housekeeping; }
long mem_stats_allocations()        { return stats_allocations; }
long mem_stats_freeings()           { return stats_freeings; }
void mem_set_verbose_mode(int verbose) { stats_verbose_mode = verbose; }


void mem_stats_take_snapshot() {
    snapshot_bytes_allocated    = stats_bytes_allocated;
    snapshot_bytes_freed        = stats_bytes_freed;
    snapshot_bytes_housekeeping = stats_bytes_housekeeping;
    snapshot_allocations        = stats_allocations;
    snapshot_freeings           = stats_freeings;
}

void mem_stats_compare_snapshot() {
    printf("Mem operation      Allocation      Freeing    Remaining\n");
    printf("Times              %10ld   %10ld   %10ld\n", 
        stats_allocations - snapshot_allocations,
        stats_freeings - snapshot_freeings,
        (stats_allocations - snapshot_allocations) - (stats_freeings - snapshot_freeings)
    );
    printf("Bytes              %10ld   %10ld   %10ld\n", 
        stats_bytes_allocated - snapshot_bytes_allocated,
        stats_bytes_freed - snapshot_bytes_freed,
        (stats_bytes_allocated - snapshot_bytes_allocated) - (stats_bytes_freed - snapshot_bytes_freed)
    );
    printf("Total times        %10ld   %10ld   %10ld\n", 
        stats_allocations, stats_freeings, (stats_allocations - stats_freeings));
    printf("Total bytes        %10ld   %10ld   %10ld\n", 
        stats_bytes_allocated, stats_bytes_freed, (stats_bytes_allocated - stats_bytes_freed));
}




