/*
    To define new object:
    - must instantiate the class


*/

#include <stdlib.h>
#include <stddef.h>
#include "objects.h"

typedef struct list_entry {
    struct list_entry *next;
    struct list_entry *prev;
    object *value;
} list_entry;

typedef struct list_object {
    BASE_OBJECT_ATTRIBUTES;
    list_entry *first;
    list_entry *last;
    int length;
} list_object;

static void initializer(object *obj, object *args, object *named_args) {
    list_object *l = (list_object *)obj;
    l->first = NULL;
    l->last = NULL;
    l->length = 0;
}

static void destructor(object *obj) {
    list_object *l = (list_object *)obj;
    // assume we deallocate all list entries...
    
}

static object *list_add(object *self, object *item) {

};
static object *list_insert(object *self, object *index, object *item) {

};
static object *list_delete(object *self, object *index) {

};

static struct type_method_definition methods[] = {
    { "add",    list_add,    0 },
    { "insert", list_insert, 0 },
    { "delete", list_delete, 0 },
    { NULL }
};

static struct type_attrib_definition attributes[] = {
    { "length", NULL, NULL, offsetof(list_object, length), TMT_INT },
    { NULL }
};

// instance of the type info
static type_object *list_type_object = &(type_object){
    .name = "list",
    .instance_size = sizeof(list_object),
    .initializer = initializer,
    .destructor = destructor,
    .methods = methods,
    .attributes = attributes
};

void make_list_type_object() {
    list_type_object.base_type = type_of_types;
    object_register_type(list_type_object);
}
