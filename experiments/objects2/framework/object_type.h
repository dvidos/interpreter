#ifndef _TYPE_OBJECT_H
#define _TYPE_OBJECT_H

#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool


// forward declarations
typedef struct object object;
typedef struct object_type object_type;

// some specific function types, used in types
typedef void (*initialize_func)(object *obj, object *args, object *named_args);
typedef void (*destruct_func)(object *obj);
typedef void (*copy_initializer_func)(object *obj, object *original);
typedef object *(*return_obj_func)(object *obj);
typedef object *(*stringifier_func)(object *obj);
typedef unsigned (*hashing_func)(object *obj);
typedef int (*compare_func)(object *a, object *b);
typedef bool (*equals_func)(object *a, object *b);
typedef object *(*call_handler_func)(object *obj, object *args, object *named_args);

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
    TAT_INT     = 1,
    TAT_BOOL    = 2,
    TAT_CONST_CHAR_PTR = 3,
    TAT_OBJECT_PTR = 4,
    TAT_READ_ONLY = 1024,
};
typedef struct type_attrib_definition {
    const char *name;
    type_attrib_getter *getter; // optional, preferred if not null
    type_attrib_setter *setter; // optional, preferred if not null
    int offset; // offsetof() the attribute in the object structure
    enum type_attrib_type tat_flags;
} type_attrib_definition;


// each object is associated with a object_type. 
// that type describes how instances behave, initialize, destruct, etc.
// the object_type also is associated with a static object_type instance, the type-type!
struct object_type {

    // base attributes allow this to impersonate a `object` struct.
    BASE_OBJECT_FIRST_ATTRIBUTES;

    // name of the class, instance_size to allocate, base type
    const char *name;
    int instance_size;
    struct object_type *base_type;

    // class wide functions to define behavior of instances
    // e.g. to_string(), to_bool(), hash(), len(), call(), serialize(), iterator() etc
    // wherever supported
    initialize_func    initializer;
    destruct_func      destructor;
    copy_initializer_func   copy_initializer;
    stringifier_func   stringifier;
    equals_func        equality_checker;
    hashing_func       hasher;
    compare_func       comparer;
    return_obj_func    iterator_factory;
    return_obj_func    iterator_next_implementation;
    call_handler_func  call_handler;

    // array of attributes and methods of the instances
    // last element in array has a NULL name
    struct type_attrib_definition *attributes;
    struct type_method_definition *methods;
};


// this is the type of a type object. an instance of type 'type'.
extern object_type *type_of_types;

// type manipulation functions
void objects_register_type(object_type *type);
object_type *objects_get_named_type(const char *name);


#endif
