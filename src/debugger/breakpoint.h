#ifndef _BREAKPOINT_H
#define _BREAKPOINT_H

#include "../containers/contained_item_info.h"


typedef struct breakpoint breakpoint;
extern contained_item_info *breakpoint_item_info;

struct breakpoint {
    contained_item_info *item_info;
    const char *filename;
    int line_no;
};

breakpoint *new_breakpoint(const char *filename, int line_no);
bool breakpoint_is_at(breakpoint *b, const char *filename, int line_no);

#endif
