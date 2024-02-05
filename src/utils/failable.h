#ifndef _FAILABLE_H
#define _FAILABLE_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdarg.h>

typedef struct failable {
    // first section same for all failables
    bool failed;
    struct failable *inner;
    const char *func;
    const char *file;
    int line;
    const char *err_msg;
    // put result last, so size does not affect uniformity
    void *result;
} failable;

failable ok();
#define failed(inner, fmt, ...)  __failed(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
failable __failed(void *inner, const char *func, const char *file, int line, const char *fmt, ...);
const char *__fail_message(const char *fmt, va_list vl);
failable *__fail_clone(void *some_failable_ptr);



/* Declares:
   - struct   `failable_<type>`  alias to failable, for descriptive function declaration
   - function `ok_<type>()`      for returning failable with strongly typed result 
   - function `__failed_<type>()`  for returning error information

   Then a macro is needed to add file/line info:
   - define failed_<type>(...)  __failed_<type>(..., __func__, __FILE__, __LINE__, ...)
*/


// for pointed types: const_char, structs, etc
#define STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(type)  \
    typedef struct failable_##type { \
        bool failed; failable *inner; const char *func; const char *file; int line; const char *err_msg; type *result; \
    } failable_##type; \
    failable_##type ok_##type(type *result); \
    failable_##type __failed_##type(void *inner, const char *func, const char *file, int line, const char *fmt, ...);

#define STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(type) \
    failable_##type ok_##type(type *result) \
        { return (failable_##type){ false, NULL, NULL, NULL, 0, NULL, result }; } \
    failable_##type __failed_##type(void *inner, const char *func, const char *file, int line, const char *fmt, ...) \
        { \
            failable_##type f = (failable_##type){ true, NULL, func, file, line, NULL }; \
            if (inner != NULL) { f.inner = __fail_clone(inner); } \
            if (fmt != NULL) { va_list(vl); va_start(vl,fmt); f.err_msg = __fail_message(fmt, vl); va_end(vl); } \
            return f; \
        }

// for value types: bool, int, float, enums etc
#define STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(type)  \
    typedef struct failable_##type { \
        bool failed; failable *inner; const char *func; const char *file; int line; const char *err_msg; type result; \
    } failable_##type; \
    failable_##type ok_##type(type result); \
    failable_##type __failed_##type(void *inner, const char *func, const char *file, int line, const char *fmt, ...);

#define STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(type) \
    failable_##type ok_##type(type result) \
        { return (failable_##type){ false, NULL, NULL, NULL, 0, NULL, result }; } \
    failable_##type __failed_##type(void *inner, const char *func, const char *file, int line, const char *fmt, ...) \
        { \
            failable_##type f = (failable_##type){ true, NULL, func, file, line, NULL }; \
            if (inner != NULL) { f.inner = __fail_clone(inner); } \
            if (fmt != NULL) { va_list(vl); va_start(vl,fmt); f.err_msg = __fail_message(fmt, vl); va_end(vl); } \
            return f; \
        }

/* some predefined failables */

typedef const char const_char;

STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(bool);
STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(int);
STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(const_char);
STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(void);

#define failed_bool(inner, fmt, ...)        __failed_bool(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define failed_int(inner, fmt, ...)         __failed_int(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define failed_const_char(inner, fmt, ...)  __failed_const_char(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void failable_print(void *some_failable);



#endif
