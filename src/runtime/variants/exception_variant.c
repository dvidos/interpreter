#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct exception_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    char *message;
    origin *origin;
    variant *inner;
} exception_instance;

static execution_outcome initialize(exception_instance *obj, variant *args, exec_context *ctx) {
    obj->message = NULL;
    obj->origin = NULL;
    obj->inner = NULL;
    return ok_outcome(NULL);
}

static void destruct(exception_instance *obj) {
    if (obj->message != NULL)
        free(obj->message);
    if (obj->inner != NULL)
        variant_drop_ref(obj->inner);
    // we don't drop origins, they are owned by tokens/AST
}

static variant *stringify(exception_instance *obj) {
    // we should append the inner exceptions recursively...
    if (obj->origin == NULL)
        return new_str_variant("%s", obj->message);
    else
        return new_str_variant("%s, at %s:%d:%d", obj->message, obj->origin->filename, obj->origin->line_no, obj->origin->column_no);
}

variant_type *exception_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "exception",
    .parent_type = NULL,
    .instance_size = sizeof(exception_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringify,
};

static char *duplicate(const char *src) {
    if (src == NULL)
        return NULL;

    char *p = malloc(strlen(src) + 1);
    strcpy(p, src);
    return p;
}

static char *format_message(const char *fmt, va_list args) {
    char temp[256];
    vsnprintf(temp, sizeof(temp), fmt, args);
    char *p = malloc(strlen(temp) + 1);
    strcpy(p, temp);
    return p;
}


// TODO: make everything have origin and call `new_exception_at()`
variant *new_exception_variant(const char *fmt, ...) {
    execution_outcome ex = variant_create(exception_type, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    exception_instance *e = (exception_instance *)ex.result;

    va_list args;
    va_start(args, fmt);
    e->message = format_message(fmt, args);
    va_end(args);

    return (variant *)e;
}

variant *new_exception_variant_at(origin *origin, variant *inner, const char *fmt, ...) {
    execution_outcome ex = variant_create(exception_type, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    exception_instance *e = (exception_instance *)ex.result;

    va_list args;
    va_start(args, fmt);
    e->message = format_message(fmt, args);
    va_end(args);
    e->origin = origin;
    
    return (variant *)e;
}
