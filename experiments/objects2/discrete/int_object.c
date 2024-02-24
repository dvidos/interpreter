#include <stdlib.h>
#include <stddef.h>
#include "str_object.h"
#include "int_object.h"

typedef struct int_object {
    BASE_OBJECT_FIRST_ATTRIBUTES;
    int value;
} int_object;

static void initialize(object *obj, object *args, object *named_args) {
    int_object *i = (int_object *)obj;
    i->value = 0;
}

static object *to_string(int_object *obj) {
    return new_str_object("%d", obj->value);
}

static object *int_add(object *self, object *item) {

};
static object *int_factorial(object *self) {

};

static struct type_method_definition methods[] = {
    { NULL }
};

static struct type_attrib_definition attributes[] = {
    { NULL }
};

// instance of the type info
object_type *int_object_type = &(object_type){
    .name = "int",
    .instance_size = sizeof(int_object),
    .initializer = (initialize_func)initialize,
    .stringifier = (stringifier_func)to_string,
    .methods = methods,
    .attributes = attributes
};

object *new_int_object(int value) {
    int_object *o = (int_object *)object_create(int_object_type, NULL, NULL);
    o->value = value;
    return (object *)o;
}

int int_object_as_int(object *obj) {
    int_object *o = (int_object *)obj;
    return o->value;
}

