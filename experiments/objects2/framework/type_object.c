#include <stdbool.h>
#include <string.h>
#include "type_object.h"


static type_object type_of_types_instance = {
    .name = "type",
};
type_object *type_of_types = &type_of_types_instance;

// a dictionary of registered types
static struct {
    // realloc'able arrays
    char **names;
    type_object **types;
    int capacity;
    int length;
} registry = { 
    .names = NULL,
    .types = NULL,
    .capacity = 0, 
    .length = 0
};


void objects_register_type(type_object *type) {
    if (registry.capacity == 0) {
        registry.capacity = 16;
        registry.names = obj_alloc(sizeof(char *) * registry.capacity);
        registry.types = obj_alloc(sizeof(type_object *) * registry.capacity);
    } else if (registry.length + 1 >= registry.capacity) {
        registry.capacity *= 2;
        registry.names = mem_realloc(registry.names, sizeof(char *) * registry.capacity);
        registry.types = mem_realloc(registry.types, sizeof(type_object *) * registry.capacity);
    }
    registry.names[registry.length] = type->name;
    registry.types[registry.length] = type;
    registry.length += 1;
}

type_object *objects_get_named_type(const char *name) {
    for (int i = 0; i < registry.length; i++) {
        if (strcmp(registry.names[i], name) == 0)
            return registry.types[i];
    }

    return NULL;
}

