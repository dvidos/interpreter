#ifndef _ERROR_H
#define _ERROR_H

#include <stdbool.h> // for bool

/*
    Poor man's error subsystem.

    One solution would be to return error objects, similar to exceptions.
    This approach is similar to C library and CPython:

    - when encountering an error, call `set_error()` with appropriate message
    - all callers should test `is_error()` after return, and return appropriately
    - who ever handles the error calls `get_error()` to get error message
    - `clear_error()` is used to remove the error, e.g. in retries.
*/

void set_error(const char *fmt, ...);
void clear_error();
bool is_error();
const char *get_error();




#endif
