#ifndef _BREAKPOINT_H
#define _BREAKPOINT_H

#include "../utils/class.h"


typedef struct breakpoint breakpoint;
struct breakpoint {
    class *class;
    const char *filename;
    int line;

    bool ephemeral;
}

#endif
