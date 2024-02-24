#include <float.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "../framework/objects.h"
#include "../discrete/str_object.h"


#define flt_equal_ish(a, b)  (fabs((a)-(b)) < 0.0000001)

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
    printf("coordinates being copy-initialized!\n");
    c->longitude = other->longitude;
    c->lattitude = other->lattitude;
}

static object *stringifier(coordinates *c) {
    return new_str_object("coordinates(%f,%f)", c->longitude, c->lattitude);
}

static bool equality_checker(coordinates *a, coordinates *b) {
    return flt_equal_ish(a->longitude, b->longitude) &&
           flt_equal_ish(a->lattitude, b->lattitude);
}

static int comparer(coordinates *a, coordinates *b) {
    if (!flt_equal_ish(a->longitude, b->longitude))
        return a->longitude < b->longitude ? -1 : 1;
    if (!flt_equal_ish(a->lattitude, b->lattitude))
        return a->lattitude < b->lattitude ? -1 : 1;
    return 0;
}

static unsigned hasher(coordinates *c) {
    return (unsigned)((c->longitude * 100000) + (c->lattitude * 100000));
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
    .equality_checker = (equals_func)equality_checker,
    .comparer = (compare_func)comparer,
    .hasher = (hashing_func)hasher,
};

static void test_class_setup() {
    objects_register_type(coordinates_type);
}

static void test_class_creation() {
    object *c = object_create(coordinates_type, NULL, NULL);
    assert(((coordinates *)c)->longitude == 101);
    assert(((coordinates *)c)->lattitude == 102);

    object *clone = object_clone(c);
    assert(((coordinates *)clone)->longitude == 101);
    assert(((coordinates *)clone)->lattitude == 102);

    assert(objects_are_equal(c, clone));
    assert(object_compare(c, clone) == 0);

    object *stringified = object_to_string(c);
    printf("object str  is '%s'\n", str_object_as_char_ptr(stringified));

    printf("object hash is 0x%x\n", object_hash(c));

    object_drop_ref(c);
    object_drop_ref(clone);
    object_drop_ref(stringified);
}



void run_objects_tests() {
    test_class_setup();

    mem_set_verbose_mode(0);
    mem_stats_take_snapshot();
    test_class_creation();
    mem_stats_compare_snapshot();
}
