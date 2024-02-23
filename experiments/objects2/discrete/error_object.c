#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "error_object.h"










type_object *error_object = &(type_object){
    .name = "error",
};

object *new_error_object(const char *msg, ...) {
    char buffer[256];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    
    return NULL;
}

const char *error_object_as_charptr(object *obj) {
    return NULL;
}
