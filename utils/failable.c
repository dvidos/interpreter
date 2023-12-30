#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "failable.h"


failable succeeded() {
    failable s = { false, NULL };
    return s;
}

failable failed(const char *err_msg_fmt, ...) {
    failable s = { true, NULL };

    va_list args;
    va_start(args, err_msg_fmt);
    s.err_msg = _format_failable_err_msg_args(err_msg_fmt, args);
    va_end(args);

    return s;
}

const char *_format_failable_err_msg_args(const char *err_msg_fmt, va_list args) {
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), err_msg_fmt, args);
    char *err_msg = malloc(strlen(buffer) + 1);
    strcpy(err_msg, buffer);
    return err_msg;
}
