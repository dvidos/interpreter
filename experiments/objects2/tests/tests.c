#include "../framework/object.h"


struct coordinates_object {
    BASE_OBJECT_ATTRIBUTES;
    double longitude;
    double lattitude;
};

struct type_object coordinates_object_type = {
    .name = "coordinates",
    .instance_size = sizeof(struct coordinates_object),
};

static void test_class_creation() {
    object *a = new_named_instance("test", NULL, NULL);
    assert(a == NULL);

    
    

    objects_register_type();
}



void run_objects_tests() {
    test_class_creation();
}