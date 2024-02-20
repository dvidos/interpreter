#include <stdlib.h>
#include <stddef.h>
#include "objects.h"

typedef struct int_object {
    BASE_OBJECT_ATTRIBUTES;
    int value;
} int_object;

static void initializer(object *obj, object *args, object *named_args) {
    int_object *i = (int_object *)obj;
    i->value = 0;
}

static void destructor(object *obj) {
    int_object *i = (int_object *)obj;
    // assume we deallocate all int entries...
    
}

static object *int_add(object *self, object *item) {

};
static object *int_factorial(object *self) {

};

static struct type_method_definition methods[] = {
    { "add",    int_add,    0 },
    { "factorial", int_factorial, 0 },
    { NULL }
};

static struct type_attrib_definition attributes[] = {
    { NULL }
};

// instance of the type info
static type_object *int_type_object = &(type_object){
    .name = "int",
    .instance_size = sizeof(int_object),
    .initializer = initializer,
    .destructor = destructor,
    .methods = methods,
    .attributes = attributes
};

void make_int_type_object() {
    int_type_object.base_type = type_of_types;
    object_register_type(int_type_object);
}
