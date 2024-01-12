#ifndef _FAILABLE_H
#define _FAILABLE_H

#include <stdbool.h>
#include <stdarg.h>

typedef struct failable {
    bool failed;
    const char *err_msg;
    union {
        void *result;
        bool bool_result;
    };
    const char *err_file;
    int err_line;
} failable;

failable ok();
#define failed(fmt, ...)  __failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
failable __failed(const char *file, int line, const char *err_msg_fmt, ...);


/* Declares:
   - struct   `failable_<type>`  with members: failed, err_msg, result
   - function `ok_<type>()`      for returning failable with strongly typed result 
   - function `failed_<type>()`  for returning failable with error message
*/
#define STRONGLY_TYPED_FAILABLE_DECLARATION(type)  \
    typedef struct failable failable_##type; \
    failable_##type ok_##type(type *result);


/* Implements:
   - function `ok_<type>()`      for returning failable with strongly typed result 
   - function `failed_<type>()`  for returning failable with error message
*/
#define STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(type) \
    failable_##type ok_##type(type *result) { \
        failable_##type s = { false, NULL, result }; \
        return s; \
    }



typedef struct failable failable_bool;
failable_bool ok_bool(bool result);

#endif
