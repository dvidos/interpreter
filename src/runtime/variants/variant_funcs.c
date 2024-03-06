#include <string.h>
#include "variant_funcs.h"
#include "../../utils/error.h"


typedef variant *(*box_int_func)(int value);
typedef variant *(*box_bool_func)(bool value);
typedef variant *(*box_const_char_ptr_func)(const char *value);
typedef int (*unbox_int_func)(variant *value);
typedef bool (*unbox_bool_func)(variant *value);
typedef const char *(*unbox_const_char_ptr_func)(variant *value);

static struct variant_methods {
    box_int_func               int_boxer;
    box_bool_func              bool_boxer;
    box_const_char_ptr_func    const_char_ptr_boxer;
    unbox_int_func             int_unboxer;
    unbox_bool_func            bool_unboxer;
    unbox_const_char_ptr_func  const_char_ptr_unboxer;
} methods;


variant *void_instance;
variant *true_instance;
variant *false_instance;
variant *zero_instance;
variant *one_instance;


void initialize_variants() {
    // the following cannot be initialized statically
    type_of_types->_type = type_of_types;
    void_type->_type = type_of_types;
    int_type->_type = type_of_types;
    str_type->_type = type_of_types;
    bool_type->_type = type_of_types;
    float_type->_type = type_of_types;


    void_instance = new_void_variant();
    true_instance = new_bool_variant(true);
    false_instance = new_bool_variant(false);
    zero_instance = new_int_variant(0);
    one_instance = new_int_variant(1);

    true_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    false_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    zero_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    one_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
}

variant *variant_create(variant_type *type, variant *args, variant *named_args) {
    variant *p = malloc(type->instance_size);
    p->_type = type;
    p->_references_count = 1; // the one we are going to return
    if (type->initializer != NULL) {
        type->initializer(p, args, named_args);
    }
    return p;
}

variant *variant_clone(variant *obj) {
    if (obj == NULL)
        return NULL;
        
    if (obj->_type->copy_initializer == NULL)
        return NULL;
    
    variant *clone = malloc(obj->_type->instance_size);
    clone->_type = obj->_type;
    clone->_references_count = 1; // the one we are going to return

    clone->_type->copy_initializer(clone, obj);
    return clone;
}

void variant_add_ref(variant *obj) {
    obj->_references_count++;
}

void variant_drop_ref(variant *obj) {
    if (obj == NULL)
        return;
    if (obj->_references_count == VARIANT_STATICALLY_ALLOCATED)
        return;
    
    obj->_references_count--;
    if (obj->_references_count > 0)
        return;
    
    // no more references to this variant
    // we can finalize and free it up.
    if (obj->_type->destructor)
        obj->_type->destructor(obj);
    
    free(obj);
}

bool variant_is(variant *obj, variant_type *type) {
    variant_type *t = obj->_type;
    int levels = 0; // avoid infinite loops
    while (t != NULL && levels++ < 100) {
        if (t == type)
            return true;
        t = t->parent_type;
    }
    return false;
}

bool variant_is_exactly(variant *obj, variant_type *type) {
    return obj->_type == type;
}

bool variant_has_attr(variant *obj, const char *name) {
    variant_type *t = obj->_type;
    if (t->attributes == NULL) return false;
    for (int i = 0; t->attributes[i].name != NULL; i++) {
        if (strcmp(t->attributes[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

variant *variant_get_attr(variant *obj, const char *name) {
    variant_type *type = obj->_type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;
        
        type_attrib_definition *def = &type->attributes[i];
        if (def->getter != NULL) {
            return def->getter(obj, name);

        } else if (def->tat_flags & TAT_VARIANT_PTR) {
            variant *obj_ptr = (variant *)(((char *)obj) + def->offset);
            variant_add_ref(obj_ptr); // the one returned
            return obj_ptr;

        } else if (def->tat_flags & TAT_INT) {
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            return methods.int_boxer(*int_ptr);

        } else if (def->tat_flags & TAT_BOOL) {
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            return methods.bool_boxer(*bool_ptr);

        } else if (def->tat_flags & TAT_CONST_CHAR_PTR) {
            const char *char_ptr = (((const char *)obj) + def->offset);
            return methods.const_char_ptr_boxer(char_ptr);

        } else {
            set_error("attribute '%s' not supported type '%d'", name, def->tat_flags);
            return NULL;
        }
    }

    set_error("attribute '%s' not found in type '%s'", name, type->name);
    return NULL;
}

variant *variant_set_attr(variant *obj, const char *name, variant *value) {
    variant_type *type = obj->_type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;

        type_attrib_definition *def = &type->attributes[i];
        if (def->tat_flags & TAT_READ_ONLY) {
            set_error("attribute '%s' is read only in type '%s'", name, type->name);
            return NULL;
        }

        if (def->setter != NULL) {
            return def->setter(obj, name, value); // error checking

        } else if (def->tat_flags & TAT_VARIANT_PTR) {
            variant **obj_ptr = (variant **)(((char *)obj) + def->offset);
            variant_add_ref(value); // the one stored
            *obj_ptr = value;

        } else if (def->tat_flags & TAT_INT) {
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            *int_ptr = methods.int_unboxer(value);

        } else if (def->tat_flags & TAT_BOOL) {
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            *bool_ptr = methods.bool_unboxer(value);

        } else if (def->tat_flags & TAT_CONST_CHAR_PTR) {
            const char **char_ptr = (const char **)(((char *)obj) + def->offset);
            *char_ptr = methods.const_char_ptr_unboxer(value);

        } else {
            set_error("attribute '%s' not supported type '%d'", def->tat_flags);
            return NULL;
        }
    }
    set_error("attribute '%s' not found in type '%s'", name, type->name);
    return NULL;
}

bool variant_has_method(variant *obj, const char *name) {
    variant_type *type = obj->_type;
    if (type->methods == NULL) return false;
    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

variant *variant_call_method(variant *obj, const char *name, variant *args, variant *named_args) {
    variant_type *type = obj->_type;
    if (type->methods == NULL)
        return false; // notify method not found
    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) == 0) {
            // should call
            return NULL;
        }
    }
    
    //return new_error_variant("method '%s' not found in type '%s'", name, type->name);
    return NULL;
}

variant *variant_to_string(variant *obj) {
    if (obj == NULL)
        return NULL; // or maybe "(null)"
    
    if (obj->_type->stringifier != NULL)
        return obj->_type->stringifier(obj);
    return NULL; // or some default?
}

bool variants_new_are_equal(variant *a, variant *b) {
    if (a == b)
        return true;
    if (a == NULL && b != NULL)
        return false;
    if (a != NULL && b == NULL)
        return false;
    
    if (a->_type != b->_type)
        return false;
    if (a->_type->equality_checker != NULL)
        return a->_type->equality_checker(a, b);
    return a == b;
}

int variant_compare(variant *a, variant *b) {
    if (a == b)
        return 0;
    if (a == NULL && b != NULL)
        return -1;
    if (a != NULL && b == NULL)
        return 1;
    
    if (a->_type != b->_type)
        return false;
    if (a->_type->comparer != NULL)
        return a->_type->comparer(a, b);
    return -1;
}

unsigned variant_hash(variant *obj) {
    if (obj == NULL)
        return 0;
    
    if (obj->_type->hasher != NULL)
        return obj->_type->hasher(obj);
    return (unsigned)(long)obj;
}

variant *variant_get_iterator(variant *obj) { // create & reset iterator to before first
    if (obj->_type->iterator_factory != NULL)
        return obj->_type->iterator_factory(obj);
    return NULL;
}

variant *variant_iterator_next(variant *obj) { // advance and get next, or return error
    if (obj->_type->iterator_next_implementation != NULL)
        return obj->_type->iterator_next_implementation(obj);
    return NULL;
}

variant *variant_call(variant *obj, variant *args, variant *named_args) {
    // here is the hard part!
    if (obj->_type->call_handler != NULL)
        return obj->_type->call_handler(obj, args, named_args);
    return NULL;
}
