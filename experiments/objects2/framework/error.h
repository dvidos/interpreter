#ifndef _ERROR_H
#define _ERROR_H

#include <stdbool.h> // for bool

/*
    Poor man's error subsystem.
    
    - when encountering an error, call `set_error()` with appropriate message
    - all callers should test `is_error()` after return, and return appropriately
    - who ever handles the error calls `get_error()` to get error message
    - `clear_error()` is used to remove the error, e.g. in retries.
*/

bool is_error();
const char *get_error();
void set_error(const char *fmt, ...);
void clear_error();



#endif
