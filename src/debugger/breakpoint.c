#include <stdlib.h>
#include <string.h>
#include "breakpoint.h"


class *breakpoint_class = &(class){
    .classdef_magic = CLASSDEF_MAGIC,
    .type_name = "breakpoint",
    .are_equal = NULL,
    .describe = NULL,
    .hash      = NULL
};

breakpoint *new_breakpoint(const char *filename, int line_no) {
    breakpoint *b = malloc(sizeof(breakpoint));
    b->class = breakpoint_class;
    b->filename = filename;
    b->line_no = line_no;
}

bool breakpoint_is_at(breakpoint *b, const char *filename, int line_no) {
    return b->line_no == line_no && strcmp(b->filename, filename) == 0;
}


