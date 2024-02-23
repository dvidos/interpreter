#include <assert.h>
#include <stdio.h>
#include "../framework/objects.h"

typedef struct coordinates coordinates;
struct coordinates {
    FIRST_OBJECT_ATTRIBUTES;
    double longitude;
    double lattitude;
};

void coordinates_initialize(coordinates *c, object *args, object *named_args) {
    printf("coordinates being initialized!\n");
    c->longitude = 101;
    c->lattitude = 102;
}

type_object *coordinates_type = &(type_object){
    .type = type_of_types,
    .references_count = OBJECT_STATICALLY_ALLOCATED,
    .name = "coordinates",
    .instance_size = sizeof(coordinates),
    .initializer = (initialize_func)coordinates_initialize,
};

static void test_class_creation() {
    objects_register_type(coordinates_type);

    coordinates *c = (coordinates *)object_create(coordinates_type, NULL, NULL);
    assert(c != NULL);
    assert(c->longitude == 101);
}



void run_objects_tests() {
    test_class_creation();
}
