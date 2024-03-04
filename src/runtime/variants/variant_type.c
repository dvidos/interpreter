#include <stdbool.h>
#include <string.h>

#include "variants.h"


variant_type *type_of_types = &(variant_type){
    .name = "type",
};

// a dictionary of registered types
static struct {
    // realloc'able arrays
    const char **names;
    variant_type **types;
    int capacity;
    int length;
} registry = { 
    .names = NULL,
    .types = NULL,
    .capacity = 0, 
    .length = 0
};

void variants_register_type(variant_type *type) {
    // setting this here, as we cannot set it statically.
    type->_type = type_of_types;

    if (registry.capacity == 0) {
        registry.capacity = 16;
        registry.names = malloc(sizeof(char *) * registry.capacity);
        registry.types = malloc(sizeof(variant_type *) * registry.capacity);
    } else if (registry.length + 1 >= registry.capacity) {
        registry.capacity *= 2;
        registry.names = realloc(registry.names, sizeof(char *) * registry.capacity);
        registry.types = realloc(registry.types, sizeof(variant_type *) * registry.capacity);
    }
    registry.names[registry.length] = type->name;
    registry.types[registry.length] = type;
    registry.length += 1;
}

variant_type *variants_get_named_type(const char *name) {
    for (int i = 0; i < registry.length; i++) {
        if (strcmp(registry.names[i], name) == 0)
            return registry.types[i];
    }

    return NULL;
}

