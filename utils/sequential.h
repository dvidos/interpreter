#ifndef _SEQUENTIAL_H
#define _SEQUENTIAL_H

struct sequential {
    void *data;
    struct sequential *next;
};

typedef struct sequential sequential;


#endif
