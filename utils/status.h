#ifndef _STATUS_H
#define _STATUS_H

#include <stdbool.h>
#include <stdarg.h>

typedef struct status {
    bool failed;
    const char *err_msg;
} status;

status status_ok();
status status_failed(const char *err_msg_fmt, ...);
const char *_status_format_err_msg_args(const char *err_msg_fmt, va_list args);


/* Declares:
   - struct   `<type>_status`    with members: failed, err_msg, data
   - function `<type>_ok()`      for returning status with strongly typed data 
   - function `<type>_failed()`  for returning status with error message
*/
#define TYPED_STATUS_DECLARATION(type)  \
    typedef struct type##_status { bool failed; const char *err_msg; type *data; } type##_status; \
    type##_status type##_ok(type *data); \
    type##_status type##_failed(const char *err_msg_fmt, ...);

/* Implements:
   - function `<type>_ok()`      for returning status with strongly typed data 
   - function `<type>_failed()`  for returning status with error message
*/
#define TYPED_STATUS_IMPLEMENTATION(type) \
    type##_status type##_ok(type *data) { \
        type##_status s = { false, NULL, data }; \
        return s; \
    } \
    type##_status type##_failed(const char *err_msg_fmt, ...) { \
        type##_status s = { false, NULL, NULL }; \
        va_list args; va_start(args, err_msg_fmt); \
        s.err_msg = _status_format_err_msg_args(err_msg_fmt, args); \
        va_end(args); \
        return s; \
    }


#endif
