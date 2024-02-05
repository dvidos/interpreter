
#include "experiments.h"




typedef struct breakpoint {
    class *c;
    const char *file;
    int line;
} breakpoint;

bool breakpoints_are_equal(breakpoint *a, breakpoint *b) {

}

const char breakpoint_to_string(breakpoint *l) {
    str_builder *sb = new_strbuilder();
    strbuilder_addf("@%s:%d", l->file, l->line);
    return strbuilder_charptr(sb);
}

class logger = &(class) {
    .name = "logger",
    .classdef_magic = CLASSDEF_MAGIC,
    .are_equal = NULL,
    .to_string = logger_to_string
};

typedef struct logger logger;



