#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct exception_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    char *message;
    char *file;
    int line;
    int column;
    variant *inner;
} exception_instance;

static execution_outcome initialize(exception_instance *obj, variant *args, variant *named_args, exec_context *ctx) {
    obj->message = NULL;
    obj->file = NULL;
    obj->line = 0;
    obj->column = 0;
    obj->inner = NULL;
    return ok_outcome(NULL);
}

static void destruct(exception_instance *obj) {
    if (obj->message != NULL)
        free(obj->message);
    if (obj->file != NULL)
        free(obj->file);
    if (obj->inner != NULL)
        variant_drop_ref(obj->inner);
}

static void copy_initialize(exception_instance *obj, exception_instance *original) {
    obj->message = malloc(strlen(original->message) + 1);
    obj->file = malloc(strlen(original->file) + 1);
    strcpy(obj->message, original->message);
    strcpy(obj->file, original->file);
    obj->line = original->line;
    obj->column = original->column;
    obj->inner = variant_clone(original->inner);
}

static variant *stringify(exception_instance *obj) {
    // we should append the inner exceptions recursively...
    return new_str_variant("%s, at %s:%d:%d", obj->message, obj->file, obj->line, obj->column);
}

static unsigned hash(exception_instance *obj) {
    if (obj == NULL || obj->message == NULL)
        return 0;
    return simple_hash(obj->message, strlen(obj->message));
}

static int compare(exception_instance *a, exception_instance *b) {
    int cmp = strcmp(a->message, b->message);
    if (cmp != 0) return cmp;
    cmp = strcmp(a->file, b->file);
    if (cmp != 0) return cmp;
    cmp = (a->line - b->line);
    if (cmp != 0) return cmp;
    return a->column - b->column;
}

static bool are_equal(exception_instance *a, exception_instance *b) {
    return strcmp(a->message, b->message) == 0
            && strcmp(a->file, b->file) == 0
            && a->line == b->line 
            && a->column == b->column;
}

variant_type *exception_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "exception",
    .parent_type = NULL,
    .instance_size = sizeof(exception_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
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


variant *new_exception_variant(const char *fmt, ...) {
    execution_outcome ex = variant_create(exception_type, NULL, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    exception_instance *e = (exception_instance *)ex.result;

    va_list args;
    va_start(args, fmt);
    e->message = format_message(fmt, args);
    va_end(args);

    return (variant *)e;
}

variant *new_exception_variant_at(const char *filename, int line, int column, variant *inner, const char *fmt, ...) {
    execution_outcome ex = variant_create(exception_type, NULL, NULL, NULL);
    if (ex.failed || ex.excepted) return NULL;
    exception_instance *e = (exception_instance *)ex.result;

    va_list args;
    va_start(args, fmt);
    e->message = format_message(fmt, args);
    va_end(args);

    e->file = duplicate(filename);
    e->line = line;
    e->column = column;
    
    return (variant *)e;
}

void exception_variant_set_source(variant *v, const char *filename, int line, int column) {
    if (v == NULL || !variant_instance_of(v, exception_type))
        return;
    exception_instance *e = (exception_instance *)v;

    if (e->file != NULL)
        free(e->file);
    
    e->file = duplicate(filename);
    e->line = line;
    e->column = column;
}
