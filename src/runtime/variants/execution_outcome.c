#include "execution_outcome.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

execution_outcome ok_outcome(variant *result) {
    return (execution_outcome){
        .result = result,
        .excepted = false,
        .exception_thrown = NULL,
        .failed = false,
        .failure_message = NULL
    };
}

execution_outcome exception_outcome(variant *exception) {
    return (execution_outcome){
        .result = NULL,
        .excepted = true,
        .exception_thrown = exception,
        .failed = false,
        .failure_message = NULL
    };
}

execution_outcome failed_outcome(const char *fmt, ...) {
    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char *msg = malloc(strlen(buffer) + 1);
    strcpy(msg, buffer);

    return (execution_outcome){
        .result = NULL,
        .excepted = false,
        .exception_thrown = NULL,
        .failed = true,
        .failure_message = msg
    };
}

