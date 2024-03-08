#include "_internal.h"
#include "../../utils/hash.h"
#include <string.h>
#include <stdio.h>


typedef struct str_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    char *buffer;
    int capacity;
    int length;
} str_instance;

static void ensure_capacity(str_instance *obj, int capacity) {
    if (obj->capacity >= capacity)
        return;
    
    while (obj->capacity < capacity)
        obj->capacity *= 2;
    
    obj->buffer = realloc(obj->buffer, obj->capacity);
}

static void initialize(str_instance *obj, variant *args, variant *named_args) {
    obj->capacity = 16;
    obj->buffer = malloc(obj->capacity);
    obj->length = 0;
    obj->buffer[0] = 0;
}

static void destruct(str_instance *obj) {
    if (obj->buffer != NULL) {
        free(obj->buffer);
    }
}

static void copy_initialize(str_instance *obj, str_instance *original) {
    obj->capacity = original->capacity;
    obj->buffer = malloc(obj->capacity);
    obj->length = original->length;
    strcpy(obj->buffer, original->buffer);
}

static variant *stringify(str_instance *obj) {
    return variant_clone((variant *)obj);
}

static unsigned hash(str_instance *obj) {
    if (obj == NULL || obj->buffer == NULL)
        return 0;
    return simple_hash(obj->buffer, obj->length);
}

static int compare(str_instance *a, str_instance *b) {
    if (a->length != b->length)
        return a->length - b->length;
    
    return strcmp(a->buffer, b->buffer);
}

static bool are_equal(str_instance *a, str_instance *b) {
    return strcmp(a->buffer, b->buffer) == 0;
}

variant_type *str_type = &(variant_type){
    ._type = NULL,
    ._references_count = VARIANT_STATICALLY_ALLOCATED,
    
    .name = "str",
    .parent_type = NULL,
    .instance_size = sizeof(str_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_str_variant(const char *fmt, ...) {
    str_instance *s = (str_instance *)variant_create(str_type, NULL, NULL);
    if (fmt == NULL) {
        ;
    } else if (strchr(fmt, '%') == NULL) {
        ensure_capacity(s, strlen(fmt) + 1);
        strcpy(s->buffer, fmt);
        s->length = strlen(fmt);
    } else {
        char temp[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(temp, sizeof(temp), fmt, args);
        va_end(args);

        ensure_capacity(s, strlen(temp) + 1);
        strcpy(s->buffer, temp);
        s->length = strlen(temp);
    }
    return (variant *)s;
}

void str_variant_append(variant *v, variant *str) {
    if (!variant_instance_of(v, str_type))
        return;
    str_instance *vi = (str_instance *)vi;

    str_instance *stringified = (str_instance *)variant_to_string(str);
    ensure_capacity(vi, vi->length + stringified->length + 1);
    strcpy(vi->buffer + vi->length, stringified->buffer);
    vi->length += stringified->length;
    variant_drop_ref((variant *)stringified);
}

const char *str_variant_as_str(variant *v) {
    if (!variant_instance_of(v, str_type))
        return NULL;
    return ((str_instance *)v)->buffer;
}
