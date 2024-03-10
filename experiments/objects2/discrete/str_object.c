#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "../framework/objects.h"

typedef struct str {
    BASE_OBJECT_FIRST_ATTRIBUTES;
    char *buffer;
    int capacity;
    int length;
} str;

static void initialize(str *s, object *args, object *named_args) {
    s->capacity = 32; // for starters
    s->buffer = malloc(s->capacity);
    s->length = 0;
    s->buffer[s->length] = '\0';
}

static void destruct(str *s) {
    if (s->buffer != NULL)
        free(s->buffer);
}

static void ensure_capacity(str *s, int capacity) {
    if (s->capacity < capacity) {
        if (s->capacity == 0)
            s->capacity = 1;
        while (s->capacity < capacity)
            s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

static void copy_initialize(str *s, str *other) {
    s->capacity = other->capacity;
    s->buffer = malloc(s->capacity);
    strcpy(s->buffer, other->buffer);
    s->length = other->length;
}

static object *stringify(object *obj) {
    return object_clone(obj);
}

static struct variant_method_definition methods[] = {
    { NULL }
};

static struct variant_attrib_definition attributes[] = {
    { "length", NULL, NULL, offsetof(str, length), VAT_INT + VAT_READ_ONLY },
    { NULL }
};

// instance of the type info
static object_type *str_type = &(object_type){
    .name = "error",
    .instance_size = sizeof(str),
    .initializer = (initialize_func)initialize,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringify,
    .methods = methods,
    .attributes = attributes,
};

object *new_str_object(const char *fmt, ...) {
    str *s = (str *)object_create(str_type, NULL, NULL);
    char temp[128];
    const char *src;

    if (fmt != NULL) {
        if (strchr(fmt, '%') == NULL) {
            src = fmt;
        } else {
            va_list args;
            va_start(args, fmt);
            vsnprintf(temp, sizeof(temp), fmt, args);
            va_end(args);
            src = temp;
        }

        ensure_capacity(s, strlen(src) + 1);
        strcpy(s->buffer, src);
        s->length = strlen(src);
    }

    return (object *)s;
}

const char *str_object_as_char_ptr(object *obj) {
    if (obj == NULL) return NULL;
    if (obj->_type != str_type) return NULL;
    return ((str *)obj)->buffer;
}

