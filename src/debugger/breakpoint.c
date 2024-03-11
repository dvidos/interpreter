#include <stdlib.h>
#include <string.h>
#include "breakpoint.h"


item_info *breakpoint_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "breakpoint",
    .are_equal = NULL,
    .describe = NULL,
    .hash      = NULL
};

breakpoint *new_breakpoint(const char *filename, int line_no) {
    breakpoint *b = malloc(sizeof(breakpoint));
    b->item_info = breakpoint_item_info;
    b->filename = filename;
    b->line_no = line_no;
}

bool breakpoint_is_at(breakpoint *b, const char *filename, int line_no) {
    return b->line_no == line_no && strcmp(b->filename, filename) == 0;
}


