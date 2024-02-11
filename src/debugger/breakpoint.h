#ifndef _BREAKPOINT_H
#define _BREAKPOINT_H

#include "../utils/class.h"


typedef struct breakpoint breakpoint;
extern class *breakpoint_class;

struct breakpoint {
    class *class;
    const char *filename;
    int line_no;
};

breakpoint *new_breakpoint(const char *filename, int line_no);
bool breakpoint_is_at(breakpoint *b, const char *filename, int line_no);

#endif
