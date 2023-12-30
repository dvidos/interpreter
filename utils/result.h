#ifndef _ERROR_H
#define _ERROR_H

#include <stdbool.h>

typedef struct result {
    bool failed;
    char *error_message;
    void *data;
} result;

result result_ok();
result result_data(void *data);
result result_failed(const char *error_message_fmt, ...);


#endif
