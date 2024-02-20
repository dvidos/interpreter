#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

// forward declaration
typedef struct object object;



// some specific function types, used in classes and elsewhere
typedef void visitor_func(object *obj);
typedef object *objobj_func(object *obj);
typedef unsigned hushing_func(object *obj);
typedef int compare_func(object *a, object *b);
typedef bool boolobjobj_func(object *a, object *b);
typedef void initialize_func(object *obj, object *args, object *named_args);
typedef object *call_func(object *obj, object *args, object *named_args);

// each object has zero or more methods. 
// they are defined in an array of this structure
typedef object *type_method_func(object *self, object *args, object *named_args);
enum type_method_flags {
    TPF_DEFAULT = 0,
    TPF_VARARGS = 1,
    TPF_NOARGS  = 2
};
typedef struct type_method_definition {
    const char *name;
    type_method_func *func;
    enum type_method_flags tpf_flags;
} type_method_definition;


// each object has zero or more attributes. 
// they can be used directly, or through getters and setters.
// they are defined in an array of this structure
typedef object *type_attrib_getter(object *self, const char *name);
typedef object *type_attrib_setter(object *self, const char *name, object *value);
enum type_attrib_type {
    TAT_DEFAULT = 0,
    TAT_CHAR    = 1,
    TAT_INT     = 2,
    TAT_LONG    = 3,
    TAT_CONST_CHAR_PTR = 4,
    TAT_READ_ONLY = 1024,
};
typedef struct type_attrib_definition {
    const char *name;
    type_attrib_getter *getter; // optional, preferred if not null
    type_attrib_setter *setter; // optional, preferred if not null
    int offset; // offsetof() the attribute in the object structure
    enum type_attrib_type tat_flags;
} type_attrib_definition;


// each object is associated with a type_object. 
// that type describes how instances behave, initialize, destruct, etc.
// the type_object also is associated with a static type_object instance, the type-type!
typedef struct type_object {

    // base attributes allow this to impersonate a `object` struct.
    BASE_OBJECT_ATTRIBUTES;

    // name of the class, instance_size to allocate, base type
    const char *name;
    int instance_size;
    type_object *base_type;

    // class wide functions to define behavior of instances
    // e.g. to_string(), to_bool(), hash(), len(), call(), serialize(), iterator() etc
    // wherever supported
    initialize_func *initializer;
    visitor_func *destructor; // should not free the object
    objobj_func *stringifier;
    boolobjobj_func *equality_checker;
    hushing_func *hasher;
    compare_func *comparer;
    objobj_func *iterator_factory;
    objobj_func *iterator_next_implementation;
    call_func *call_implementation;

    // array of attributes and methods of the instances
    // last element in array has a NULL name
    struct type_attrib_definition **attributes;
    struct type_method_definition **methods;
} type_object;

// this is the type of a type object. an instance of type 'type'.
extern type_object *type_of_types;


// type manipulation functions
void objects_register_type(type_object *type);
type_object *objects_get_named_type(const char *name);

