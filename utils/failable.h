#ifndef _FAILABLE_H
#define _FAILABLE_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdarg.h>

typedef struct failable {
    bool failed;
    void *result;
    const char *err_msg;
    const char *bad_file;
    int bad_line;
} failable;

failable ok();
#define failed(fmt, ...)  __failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
failable __failed(const char *file, int line, const char *err_msg_fmt, ...);
const char *__fail_message(const char *fmt, va_list vl);



/* Declares:
   - struct   `failable_<type>`  alias to failable, for descriptive function declaration
   - function `ok_<type>()`      for returning failable with strongly typed result 
   - function `failed_<type>()`  for returning error information
*/



// for pointed types: const_char, structs, etc
#define STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(type)  \
    typedef struct failable_##type { bool failed; type *result; const char *err_msg; const char *bad_file; int bad_line; } failable_##type; \
    failable_##type ok_##type(type *result); \
    failable_##type failed_##type(const char *fmt, ...);

#define STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(type) \
    failable_##type ok_##type(type *result) \
        { return (failable_##type){ false, result }; } \
    failable_##type failed_##type(const char *fmt, ...) \
        { va_list(vl);va_start(vl,fmt); \
          failable_##type f = (failable_##type){ true, NULL, __fail_message(fmt, vl) }; \
          va_end(vl); return f; }

// for value types: bool, int, float etc
#define STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(type)  \
    typedef struct failable_##type { bool failed; type result; const char *err_msg; const char *bad_file; int bad_line; } failable_##type; \
    failable_##type ok_##type(type result); \
    failable_##type failed_##type(const char *fmt, ...);

#define STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(type) \
    failable_##type ok_##type(type result) \
        { return (failable_##type){ false, result }; } \
    failable_##type failed_##type(const char *fmt, ...) \
        { va_list(vl);va_start(vl,fmt); \
          failable_##type f = (failable_##type){ true, (type)0, __fail_message(fmt, vl) }; \
          va_end(vl); return f; }


/* some predefined failables */

typedef const char const_char;

STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(bool);
STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(int);
STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(const_char);


#endif
