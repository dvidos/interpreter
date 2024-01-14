#ifndef _FAILABLE_H
#define _FAILABLE_H

#include <stdbool.h>
#include <stdarg.h>

typedef struct failable {
    bool failed;
    const char *err_msg;
    void *result;
    const char *bad_file;
    int bad_line;
} failable;

failable ok();
#define failed(fmt, ...)  __failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
failable __failed(const char *file, int line, const char *err_msg_fmt, ...);



/* Declares:
   - struct   `failable_<type>`  alias to failable, for descriptive function declaration
   - function `ok_<type>()`      for returning failable with strongly typed result 
*/
#define STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(type)  \
    typedef struct failable failable_##type; \
    failable_##type ok_##type(type *result);

#define STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(type)  \
    typedef struct failable failable_##type; \
    failable_##type ok_##type(type result);


/* Implements:
   - function `ok_<type>()`      for returning failable with strongly typed result 
*/
#define STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(type) \
    failable_##type ok_##type(type *result) \
        { return (failable_##type){ false, NULL, (void *)result }; }

#define STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(type) \
    failable_##type ok_##type(type result) \
        { return (failable_##type){ false, NULL, (void *)(size_t)result }; }


/* some predefined failables */

typedef const char const_char;

STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(bool);
STRONGLY_TYPED_FAILABLE_VAL_DECLARATION(int);
STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(const_char);


#endif
