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



inline failable_bool ok_bool(bool result) {
    return (failable_bool){ false, NULL, (void *)result };
}

inline failable_constcharptr ok_constcharptr(const char *result) {
    return (failable_constcharptr){ false, NULL, (void *)result };
}