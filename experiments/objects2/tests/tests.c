#include <assert.h>
#include "../framework/objects.h"


struct coordinates_object {
    FIRST_OBJECT_ATTRIBUTES;
    double longitude;
    double lattitude;
};

type_object *coordinates_object_type = &(type_object){
    .name = "coordinates",
    .instance_size = sizeof(struct coordinates_object),
};

static void test_class_creation() {
    object *a = new_named_instance("test", NULL, NULL);
    assert(a == NULL);

    objects_register_type(coordinates_object_type);
}



void run_objects_tests() {
    test_class_creation();
}
