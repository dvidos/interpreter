#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "failable.h"


failable ok() {
    failable s = { false };
    return s;
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
    s.err_file = file;
    s.err_line = line;
    
    return s;
}



failable_bool ok_bool(bool result) {
    failable s = { false, NULL, (void *)result };
    return s;
}
