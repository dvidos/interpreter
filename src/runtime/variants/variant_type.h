#ifndef _VARIANT_TYPE_H
#define _VARIANT_TYPE_H

#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

#include "../../containers/_module.h"

#include "variant_base.h"
#include "execution_outcome.h"


typedef struct exec_context exec_context;

// some specific function types, used in types
typedef void (*initialize_func)(variant *obj, variant *args, variant *named_args);
typedef void (*destruct_func)(variant *obj);
typedef void (*copy_initializer_func)(variant *obj, variant *original);
typedef variant *(*return_obj_func)(variant *obj);
typedef variant *(*stringifier_func)(variant *obj);
typedef unsigned (*hashing_func)(variant *obj);
typedef int (*compare_func)(variant *a, variant *b);
typedef bool (*equals_func)(variant *a, variant *b);

typedef execution_outcome (*iterator_next_func)(variant *obj);
typedef execution_outcome (*call_handler_func)(variant *obj, list *args, dict *named_args, exec_context *ctx);
typedef execution_outcome (*get_element_func)(variant *obj, variant *index);
typedef execution_outcome (*set_element_func)(variant *obj, variant *index, variant *value);

// each variant has zero or more methods. 
// they are defined in an array of this structure
typedef execution_outcome (*variant_method_handler_func)(variant *this, list *args, dict *named_args, exec_context *ctx);
enum variant_method_flags {
    VMF_DEFAULT = 0,
    VMF_VARARGS = 1,
    VMF_NOARGS  = 2
};
typedef struct variant_method_definition {
    const char *name;
    variant_method_handler_func handler;
    enum variant_method_flags tpf_flags;
} variant_method_definition;


// each variant has zero or more attributes. 
// they can be used directly, or through getters and setters.
// they are defined in an array of this structure
typedef execution_outcome variant_attrib_getter(variant *this, const char *name);
typedef execution_outcome variant_attrib_setter(variant *this, const char *name, variant *value);
enum variant_attrib_type {
    VAT_DEFAULT = 0,
    VAT_INT     = 1,
    VAT_BOOL    = 2,
    VAT_CONST_CHAR_PTR = 3,
    VAT_VARIANT_PTR = 4,
    VAT_READ_ONLY = 1024,
};
typedef struct variant_attrib_definition {
    const char *name;
    variant_attrib_getter *getter; // optional, preferred if not null
    variant_attrib_setter *setter; // optional, preferred if not null
    int offset; // offsetof() the attribute in the variant structure
    enum variant_attrib_type tat_flags;
} variant_attrib_definition;


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
    initialize_func        initializer;
    destruct_func          destructor;
    copy_initializer_func  copy_initializer;
    stringifier_func       stringifier;
    equals_func            equality_checker;
    hashing_func           hasher;
    compare_func           comparer;
    return_obj_func        iterator_factory;
    iterator_next_func     iterator_next_implementation;
    call_handler_func      call_handler;
    get_element_func       get_element;
    set_element_func       set_element;

    // array of attributes and methods of the instances
    // last element in array has a NULL name
    struct variant_attrib_definition *attributes;
    struct variant_method_definition *methods;
};


// this is the type of a type variant. an instance of type 'type'.
extern variant_type *type_of_types;

// type manipulation functions
void variants_register_type(variant_type *type);
variant_type *variants_get_named_type(const char *name);


#endif
