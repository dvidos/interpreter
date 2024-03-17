#include "mem.h"
#include "origin.h"


static origin internal_origin_singleton = {
    .filename = "(internal code)",
    .line_no = 0,
    .column_no = 0
};

origin *new_origin(const char *filename, int line_no, int column_no) {
    origin *p = malloc(sizeof(origin));
    p->filename = filename;
    p->line_no = line_no;
    p->column_no = column_no;
    return p;
}

origin *internal_origin() {
    return &internal_origin_singleton;
}
