#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "result.h"


result result_ok() {
    result r;
    r.failed = false;
    r.error_message = NULL;
    r.data = NULL;
    return r;
}

result result_data(void *data) {
    result r;
    r.failed = false;
    r.error_message = NULL;
    r.data = data;
    return r;
}

result result_failed(const char *error_message_fmt, ...) {
    char buffer[256];
    va_list args;

    va_start(args, error_message_fmt);
    vsnprintf(buffer, sizeof(buffer), error_message_fmt, args);
    va_end(args);

    result r;
    r.failed = true;
    r.error_message = malloc(strlen(buffer) + 1);
    strcpy(r.error_message, buffer);
    r.data = NULL;
    return r;
}
