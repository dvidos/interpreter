
#include "experiments.h"




typedef struct breakpoint {
    class *c;
    const char *file;
    int line;
} breakpoint;

bool breakpoints_are_equal(breakpoint *a, breakpoint *b) {

}

const void breakpoint_describe(breakpoint *l, str_builder *sb) {
    strbuilder_addf("@%s:%d", l->file, l->line);
}

class logger = &(class) {
    .name = "logger",
    .classdef_magic = CLASSDEF_MAGIC,
    .are_equal = NULL,
    .describe = logger_describe
};

typedef struct logger logger;



