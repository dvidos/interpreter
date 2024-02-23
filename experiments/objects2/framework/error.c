#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char error_message[256] = {0};

void set_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(error_message, sizeof(error_message), fmt, args);
    va_end(args);
}

void clear_error() {
    error_message[0] = '\0';
}

bool is_error() {
    return error_message[0] != 0;
}

const char *get_error() {
    return error_message;
}

