#ifndef _POOL_H
#define _POOL_H

#include <stdbool.h>


typedef struct pool pool;

pool *popen();
void *palloc(pool *p, int size);
void pclose(pool *p);

bool pool_self_diagnostics();



#endif
