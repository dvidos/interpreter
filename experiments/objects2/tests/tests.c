#include <assert.h>
#include <stdio.h>
#include "../framework/objects.h"
#include "../discrete/str_object.h"

typedef struct coordinates coordinates;
struct coordinates {
    FIRST_OBJECT_ATTRIBUTES;
    double longitude;
    double lattitude;
};

static void initialize(coordinates *c, object *args, object *named_args) {
    printf("coordinates being initialized!\n");
    c->longitude = 101;
    c->lattitude = 102;
}

static void destruct(coordinates *c) {
    printf("coordinates being destructed!\n");
}

static void copy_initializer(coordinates *c, coordinates *other) {
    c->longitude = other->longitude;
    c->lattitude = other->lattitude;
}

static object *stringifier(coordinates *c) {
    return new_str_object("coordinates(%f,%f)", c->longitude, c->lattitude);
}

object_type *coordinates_type = &(object_type){
    // .type = type_of_types,
    ._references_count = OBJECT_STATICALLY_ALLOCATED,
    .name = "coordinates",
    .instance_size = sizeof(coordinates),
    .initializer = (initialize_func)initialize,
    .copy_initializer = (copy_initializer_func)copy_initializer,
    .destructor = (destruct_func)destruct,
    .stringifier = (stringifier_func)stringifier,
};

static void test_class_creation() {
    objects_register_type(coordinates_type);

    object *c = object_create(coordinates_type, NULL, NULL);
    assert(((coordinates *)c)->longitude == 101);
    assert(((coordinates *)c)->lattitude == 102);

    object *clone = object_clone(c);
    assert(((coordinates *)clone)->longitude == 101);
    assert(((coordinates *)clone)->lattitude == 102);

    object *stringified = object_to_string(c);
    printf("object is: '%s'\n", str_object_as_char_ptr(stringified));

    object_drop_ref(c);
    object_drop_ref(clone);
    object_drop_ref(stringified);
}



void run_objects_tests() {
    test_class_creation();
}
