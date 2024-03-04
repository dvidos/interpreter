#include "str_object.h"

typedef struct str_object {
    BASE_OBJECT_FIRST_ATTRIBUTES;
    char *buffer;
    int capacity;
    int length;
} str_object;

static void initialize(object *obj, object *args, object *named_args) {
    str_object *s = (str_object *)obj;
    s->capacity = 32;
    s->buffer = malloc(s->capacity);
    s->buffer[0] = 0;
    s->length = 0;
}

static object *to_string(str_object *obj) {
    return obj; // or clone?
}


static struct type_method_definition methods[] = {
    // left, right, substr, find, toupper, tolower, etc.
    { NULL }
};

static struct type_attrib_definition attributes[] = {
    // read only length
    { NULL }
};

// instance of the type info
object_type *str_type = &(object_type){
    .name = "str",
    .base_type = NULL,
    .instance_size = sizeof(str_object),

    .initializer = (initialize_func)initialize,
    .stringifier = (stringifier_func)to_string,

    .methods = methods,
    .attributes = attributes
};

object *new_str_object(const char *value) {
    str_object *s = (str_object *)object_create(str_type, NULL, NULL);
    // ensure capacity
    // copy
    return (object *)s;
}

const char *str_object_as_str(object *obj) {
    str_object *s = (str_object *)obj;
    return s->buffer;
}
