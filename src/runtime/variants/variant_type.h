#ifndef _VARIANT_TYPE_H
#define _VARIANT_TYPE_H

#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

#include "variant_base.h"
#include "execution_outcome.h"

#include "int_variant.h"
#include "str_variant.h"
#include "bool_variant.h"
#include "float_variant.h"


// some specific function types, used in types
typedef void (*initialize_func)(variant *obj, variant *args, variant *named_args);
typedef void (*destruct_func)(variant *obj);
typedef void (*copy_initializer_func)(variant *obj, variant *original);
typedef variant *(*return_obj_func)(variant *obj);
typedef variant *(*stringifier_func)(variant *obj);
typedef unsigned (*hashing_func)(variant *obj);
typedef int (*compare_func)(variant *a, variant *b);
typedef bool (*equals_func)(variant *a, variant *b);
typedef variant *(*call_handler_func)(variant *obj, variant *args, variant *named_args);
typedef execution_outcome (*get_element_func)(variant *obj, variant *index);
typedef execution_outcome (*set_element_func)(variant *obj, variant *index, variant *value);

// each variant has zero or more methods. 
// they are defined in an array of this structure
typedef variant *type_method_func(variant *self, variant *args, variant *named_args);
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


// each variant has zero or more attributes. 
// they can be used directly, or through getters and setters.
// they are defined in an array of this structure
typedef variant *type_attrib_getter(variant *self, const char *name);
typedef variant *type_attrib_setter(variant *self, const char *name, variant *value);
enum type_attrib_type {
    TAT_DEFAULT = 0,
    TAT_INT     = 1,
    TAT_BOOL    = 2,
    TAT_CONST_CHAR_PTR = 3,
    TAT_VARIANT_PTR = 4,
    TAT_READ_ONLY = 1024,
};
typedef struct type_attrib_definition {
    const char *name;
    type_attrib_getter *getter; // optional, preferred if not null
    type_attrib_setter *setter; // optional, preferred if not null
    int offset; // offsetof() the attribute in the variant structure
    enum type_attrib_type tat_flags;
} type_attrib_definition;


// each variant is associated with a variant_type. 
// that type describes how instances behave, initialize, destruct, etc.
// the variant_type also is associated with a static variant_type instance, the type-type!
struct variant_type {

    // base attributes allow this to impersonate a `variant` struct.
    BASE_VARIANT_FIRST_ATTRIBUTES;

    // name of the class, instance_size to allocate, base type
    const char *name;
    int instance_size;
    struct variant_type *parent_type;

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
    get_element_func   get_element;
    set_element_func   set_element;

    // array of attributes and methods of the instances
    // last element in array has a NULL name
    struct type_attrib_definition *attributes;
    struct type_method_definition *methods;
};


// this is the type of a type variant. an instance of type 'type'.
extern variant_type *type_of_types;

// type manipulation functions
void variants_register_type(variant_type *type);
variant_type *variants_get_named_type(const char *name);


#endif
