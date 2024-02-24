#include <assert.h>
#include <stdio.h>
#include "../framework/objects.h"
#include "../discrete/str_object.h"
#include "../discrete/int_object.h"


/*  function make_generator(start, step) {
        return function() {
            value = start;
            start += step;
            return value;
        };
    }
*/

typedef struct num_generator num_generator;
struct num_generator {
    BASE_OBJECT_FIRST_ATTRIBUTES;
    int next_number;
    int step;
};

static void initialize(num_generator *g, object *args, object *named_args) {
    g->next_number = 0;
    g->step = 2;
}

object *call_handler(num_generator *g, object *args, object *named_args) {
    int value = g->next_number;
    g->next_number += g->step;
    return new_int_object(value);
}

object_type *num_generator_type = &(object_type){
    // .type = type_of_types,
    ._references_count = OBJECT_STATICALLY_ALLOCATED,
    .name = "num_generator",
    .instance_size = sizeof(num_generator),
    .initializer = (initialize_func)initialize,
    .call_handler = (call_handler_func)call_handler
};

void make_call(object *generator) {
    object *number = object_call(generator, NULL, NULL);
    object *representation = object_to_string(number);

    printf("number is %s\n", str_object_as_char_ptr(representation));

    object_drop_ref(number);
    object_drop_ref(representation);
}

void test_callables_class() {
    // root function, i.e. round()
    // a user defined function (e.g. "loadCarton()")
    // an object/class method with the 'this' value
    // an anonymous function (e.g. "function(x){ return x % 2 == 0; }")

    // a closure having captured some values of its environment
    // this allows for making function factories

    // a closure modifying captured, i.e. calling it twice.

    // we implement the above via:
    // - methods in an object (with "this")
    // - the callable_object.
    // essentially, both objects and closures have (modifiable) data and code.

    // the way it is descibed here (https://groups.seas.harvard.edu/courses/cs153/2018fa/lectures/Lec12-Functions.pdf) 
    // is that closure functions also take the environment as argument, 
    // same as the 'this' object
    
    // an example can be a parametric number generator, say (start,step)
    object *generator = object_create(num_generator_type, NULL, NULL);

    make_call(generator);
    make_call(generator);
    make_call(generator);
    make_call(generator);

    object_drop_ref(generator);
}
