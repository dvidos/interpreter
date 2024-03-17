#ifndef _VARIANT_TYPE_H
#define _VARIANT_TYPE_H

#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

#include "../../utils/origin.h"
#include "../../containers/_module.h"

#include "variant_base.h"
#include "execution_outcome.h"


typedef struct exec_context exec_context;

// some specific function types, used in types
typedef execution_outcome (*initialize_func)(variant *obj, variant *args, exec_context *ctx);
typedef void (*destruct_func)(variant *obj);
typedef void (*copy_initializer_func)(variant *obj, variant *original);
typedef variant *(*return_obj_func)(variant *obj);
typedef variant *(*stringifier_func)(variant *obj);
typedef unsigned (*hashing_func)(variant *obj);
typedef int (*compare_func)(variant *a, variant *b);
typedef bool (*equals_func)(variant *a, variant *b);

typedef execution_outcome (*iterator_next_func)(variant *obj);
typedef execution_outcome (*call_handler_func)(variant *obj, list *args, variant *this_obj, origin *call_origin, exec_context *ctx);
typedef execution_outcome (*get_element_func)(variant *obj, variant *index);
typedef execution_outcome (*set_element_func)(variant *obj, variant *index, variant *value);


// each variant has zero or more methods. 
// they are defined in an array of this structure
typedef struct variant_method_definition variant_method_definition;
typedef execution_outcome (*variant_method_handler_func)(variant *this, variant_method_definition *method, list *args, origin *call_origin, exec_context *ctx);
enum variant_method_flags {
    VMF_DEFAULT = 0,
    VMF_VARARGS = 1,
    VMF_NOARGS  = 2,
    VMF_PUBLIC  = 1024
};
struct variant_method_definition {
    const char *name;
    variant_method_handler_func handler;
    enum variant_method_flags vmf_flags;
    void *ast_node; // for AST based methods
};


// each variant has zero or more attributes. 
// they can be used directly, or through getters and setters.
// they are defined in an array of this structure
typedef struct variant_attrib_definition variant_attrib_definition;
typedef execution_outcome variant_attrib_getter(variant *this, variant_attrib_definition *attrib);
typedef execution_outcome variant_attrib_setter(variant *this, variant_attrib_definition *attrib, variant *value);
enum variant_attrib_flags {
    VAF_DEFAULT = 0,
    VAF_INT_VARIANT     = 1,
    VAF_BOOL_VARIANT    = 2,
    VAF_STR_VARIANT = 3,
    VAF_ANY_VARIANT = 4,
    VAF_PUBLIC = 1024,
    VAF_READ_ONLY = 2048,
};
typedef struct variant_attrib_definition {
    const char *name;
    variant_attrib_getter *getter; // optional, preferred if not null
    variant_attrib_setter *setter; // optional, preferred if not null
    int offset; // offsetof() the attribute in the variant structure
    enum variant_attrib_flags vaf_flags;
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

    // the AST node that created this type.
    // most probably a statement, maybe an expression in the future
    void *ast_node;

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
    variant_attrib_definition *attributes;
    variant_method_definition *methods;
};


// this is the type of a type variant. an instance of type 'type'.
extern variant_type *type_of_types;

// type manipulation functions
void variants_register_type(variant_type *type);
variant_type *variants_get_named_type(const char *name);


#endif
