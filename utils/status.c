#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "status.h"


status status_ok() {
    status s;
    s.failed = false;
    s.error_message = NULL;
    s.data = NULL;
    return s;
}

status status_data(void *data) {
    status s;
    s.failed = false;
    s.error_message = NULL;
    s.data = data;
    return s;
}

status status_failed(const char *error_message_fmt, ...) {
    char buffer[256];
    va_list args;

    va_start(args, error_message_fmt);
    vsnprintf(buffer, sizeof(buffer), error_message_fmt, args);
    va_end(args);

    status s;
    s.failed = true;
    s.error_message = malloc(strlen(buffer) + 1);
    strcpy(s.error_message, buffer);
    s.data = NULL;
    return s;
}
