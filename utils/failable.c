#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "failable.h"


failable ok() {
    return (failable){ false };
}

failable __failed(const char *file, int line, const char *err_msg_fmt, ...) {
    failable s = { true, NULL };

    char buffer[256];
    va_list args;
    va_start(args, err_msg_fmt);
    vsnprintf(buffer, sizeof(buffer), err_msg_fmt, args);
    va_end(args);
    char *p = malloc(strlen(buffer) + 1);
    strcpy(p, buffer);

    s.err_msg = p;
    s.bad_file = file;
    s.bad_line = line;
    
    return s;
}


STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(bool);
STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(int);
STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(const_char);

