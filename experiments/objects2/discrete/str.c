#include "../objects.h"
#include "../mem.h"

typedef struct str_object {
    BASE_OBJECT_ATTRIBUTES;
    const char *buffer;
    int capacity;
    int length;
} str_object;

static void initializer(object *obj, object *args, object *named_args) {
    str_object *e = (str_object *)obj;
    // ...
}

static void destructor(object *obj) {
    str_object *e = (str_object *)obj;
    if (e->buffer != NULL)
        mem_free(e->buffer);
}

static struct type_method_definition methods[] = {
    { NULL }
};

static struct type_attrib_definition attributes[] = {
    { "buffer", NULL, NULL, offsetof(str_object, buffer), TMT_CONST_CHAR_PTR },
    { NULL }
};

static object *str_stringify(object *obj) {
    // what about references?
    return obj;
}

// instance of the type info
static type_object error_type_object = {
    .name = "error",
    .instance_size = sizeof(str_object),
    .initializer = initializer,
    .destructor = destructor,
    .methods = methods,
    .attributes = attributes,
    .stringifier = str_stringify
};

object *new_str_object_from_char_ptr(const char *ptr);
const char *ptr str_object_as_char_ptr(object *obj);
