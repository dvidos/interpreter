#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

// objects depend on type, not the other way around
// most files should include this header anyway (objects.h)
#include "type_object.h" 


// very poor man's error subsystem
void set_error(const char *fmt, ...);
void clear_error();
bool is_error();
const char *get_error();



// the base object attributes are in a define,
// to allow them to be placed in the same place, in child classes
#define BASE_OBJECT_ATTRIBUTES    \
            struct type_object *type;  \
            int references_count

#define OBJECT_STATICALLY_ALLOCATED   (-2)
// all objects can be cast to this pointer
// essentially, they can all pretend to be this structure
// check the "type" attribute, to find which "subclass" they are
typedef struct object { // all objects children of this
    BASE_OBJECT_ATTRIBUTES;
} object;


// call this to create a new instance
object *new_typed_instance(type_object *type,     object *args, object *named_args);
object *new_named_instance(const char *type_name, object *args, object *named_args);
void object_destruct_and_free(object *obj);

// references count, ala python.
void object_incr_ref_count(object *obj);
void object_decr_ref_count(object *obj);

// type checks
bool object_is(object *obj, type_object *type);          // type or a subtype of it
bool object_is_exactly(object *obj, type_object *type);  // type or a subtype of it


// call these to manipulate properties on an object
bool    object_has_attr(object *obj, const char *name);
object *object_get_attr(object *obj, const char *name);
object *object_set_attr(object *obj, const char *name, object *value);

// call these to manipulate methods on an object
bool object_has_method(object *obj, const char *name);
object *object_call_method(object *obj, const char *name, object *args, object *named_args);

// a few utilitiy methods without knowing the object type
object *object_to_string(object *obj);
bool objects_are_equal(object *a, object *b);
int object_compare(object *a, object *b);
unsigned object_hash(object *obj);
object *object_get_iterator(object *obj); // create & reset iterator to before first
object *object_iterator_next(object *obj); // advance and get next, or return error
object *object_call(object *obj, object *args, object *named_args);





