#include "mem.h"
#include "origin.h"

origin *new_origin(const char *filename, int line_no, int column_no) {
    origin *p = malloc(sizeof(origin));
    p->filename = filename;
    p->line_no = line_no;
    p->column_no = column_no;
    return p;
}


