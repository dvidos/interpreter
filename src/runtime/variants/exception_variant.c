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

static void initialize(exception_instance *obj, variant *args, variant *named_args) {
    obj->message = NULL;
    obj->file = NULL;
    obj->line = 0;
    obj->column = 0;
    obj->inner = NULL;
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

variant *new_exception_variant(const char *filename, int line, int column, variant *inner, const char *fmt, ...) {
    exception_instance *e = (exception_instance *)variant_create(exception_type, NULL, NULL);

    char temp[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);
    e->message = malloc(strlen(temp) + 1);
    strcpy(e->message, temp);

    e->file = malloc(strlen(filename) + 1);
    strcpy(e->file, filename);
    e->line = line;
    e->column = column;
    e->inner = variant_clone(inner);
    
    return (variant *)e;
}