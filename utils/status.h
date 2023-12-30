#ifndef _STATUS_H
#define _STATUS_H

#include <stdbool.h>

typedef struct status {
    bool failed;
    char *error_message;
    void *data;
} status;


status status_ok();
status status_data(void *data);
status status_failed(const char *error_message_fmt, ...);


#endif
