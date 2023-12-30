#ifndef _FAILABLE_H
#define _FAILABLE_H

#include <stdbool.h>
#include <stdarg.h>

typedef struct failable {
    bool failed;
    const char *err_msg;
} failable;

failable succeeded();
failable failed(const char *err_msg_fmt, ...);
const char *_format_failable_err_msg_args(const char *err_msg_fmt, va_list args);


/* Declares:
   - struct   `failable_<type>`  with members: failed, err_msg, result
   - function `ok_<type>()`      for returning failable with strongly typed result 
   - function `failed_<type>()`  for returning failable with error message
*/
#define STRONGLY_TYPED_FAILABLE_DECLARATION(type)  \
    typedef struct failable_##type { bool failed; const char *err_msg; type *result; } failable_##type; \
    failable_##type ok_##type(type *result); \
    failable_##type failed_##type(const char *err_msg_fmt, ...);

/* Implements:
   - function `<type>_ok()`      for returning failable with strongly typed result 
   - function `<type>_failed()`  for returning failable with error message
*/
#define STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(type) \
    failable_##type ok_##type(type *result) { \
        failable_##type s = { false, NULL, result }; \
        return s; \
    } \
    failable_##type failed_##type(const char *err_msg_fmt, ...) { \
        failable_##type s = { true, NULL, NULL }; \
        va_list args; va_start(args, err_msg_fmt); \
        s.err_msg = _format_failable_err_msg_args(err_msg_fmt, args); \
        va_end(args); \
        return s; \
    }


#endif
