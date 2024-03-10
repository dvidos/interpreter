#include <string.h>
#include "objects.h"

typedef object *(*box_int_func)(int value);
typedef object *(*box_bool_func)(bool value);
typedef object *(*box_const_char_ptr_func)(const char *value);
typedef int (*unbox_int_func)(object *value);
typedef bool (*unbox_bool_func)(object *value);
typedef const char *(*unbox_const_char_ptr_func)(object *value);

static struct object_methods {
    box_int_func               int_boxer;
    box_bool_func              bool_boxer;
    box_const_char_ptr_func    const_char_ptr_boxer;
    unbox_int_func             int_unboxer;
    unbox_bool_func            bool_unboxer;
    unbox_const_char_ptr_func  const_char_ptr_unboxer;
} methods;


object *object_create(object_type *type, object *args, object *named_args) {
    object *p = malloc(type->instance_size);
    p->_type = type;
    p->_references_count = 1; // the one we are going to return
    if (type->initializer != NULL) {
        type->initializer(p, args, named_args);
    }
    return p;
}

object *object_clone(object *obj) {
    if (obj->_type->copy_initializer == NULL)
        return NULL;
    
    object *clone = malloc(obj->_type->instance_size);
    clone->_type = obj->_type;
    clone->_references_count = 1; // the one we are going to return

    clone->_type->copy_initializer(clone, obj);
    return clone;
}

void object_add_ref(object *obj) {
    if (obj == NULL)
        return;
    if (obj->_references_count == OBJECT_STATICALLY_ALLOCATED)
        return;
    
    obj->_references_count++;
}

void object_drop_ref(object *obj) {
    if (obj == NULL)
        return;
    if (obj->_references_count == OBJECT_STATICALLY_ALLOCATED)
        return;
    
    obj->_references_count--;
    if (obj->_references_count > 0)
        return;
    
    // no more references to this object
    // we can finalize and free it up.
    if (obj->_type->destructor)
        obj->_type->destructor(obj);
    
    free(obj);
}

bool object_is(object *obj, object_type *type) {
    object_type *t = obj->_type;
    int levels = 0; // avoid infinite loops
    while (t != NULL && levels++ < 100) {
        if (t == type)
            return true;
        t = t->base_type;
    }
    return false;
}

bool object_is_exactly(object *obj, object_type *type) {
    return obj->_type == type;
}

bool object_has_attr(object *obj, const char *name) {
    object_type *t = obj->_type;
    if (t->attributes == NULL) return false;
    for (int i = 0; t->attributes[i].name != NULL; i++) {
        if (strcmp(t->attributes[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

object *object_get_attr(object *obj, const char *name) {
    object_type *type = obj->_type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;
        
        variant_attrib_definition *def = &type->attributes[i];
        if (def->getter != NULL) {
            return def->getter(obj, name);

        } else if (def->tat_flags & TAT_OBJECT_PTR) {
            object *obj_ptr = (object *)(((char *)obj) + def->offset);
            object_add_ref(obj_ptr); // the one returned
            return obj_ptr;

        } else if (def->tat_flags & VAT_INT) {
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            return methods.int_boxer(*int_ptr);

        } else if (def->tat_flags & VAT_BOOL) {
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            return methods.bool_boxer(*bool_ptr);

        } else if (def->tat_flags & VAT_CONST_CHAR_PTR) {
            const char *char_ptr = (((const char *)obj) + def->offset);
            return methods.const_char_ptr_boxer(char_ptr);

        } else {
            set_error("attribute '%s' not supported type '%d'", def->tat_flags);
            return NULL;
        }
    }

    set_error("attribute '%s' not found in type '%s'", name, type->name);
    return NULL;
}

object *object_set_attr(object *obj, const char *name, object *value) {
    object_type *type = obj->_type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;

        variant_attrib_definition *def = &type->attributes[i];
        if (def->tat_flags & VAT_READ_ONLY) {
            set_error("attribute '%s' is read only in type '%s'", name, type->name);
            return NULL;
        }

        if (def->setter != NULL) {
            return def->setter(obj, name, value); // error checking

        } else if (def->tat_flags & TAT_OBJECT_PTR) {
            object **obj_ptr = (object **)(((char *)obj) + def->offset);
            object_add_ref(value); // the one stored
            *obj_ptr = value;

        } else if (def->tat_flags & VAT_INT) {
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            *int_ptr = methods.int_unboxer(value);

        } else if (def->tat_flags & VAT_BOOL) {
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            *bool_ptr = methods.bool_unboxer(value);

        } else if (def->tat_flags & VAT_CONST_CHAR_PTR) {
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

bool object_has_method(object *obj, const char *name) {
    object_type *type = obj->_type;
    if (type->methods == NULL) return false;
    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

object *object_call_method(object *obj, const char *name, object *args, object *named_args) {
    object_type *type = obj->_type;
    if (type->methods == NULL)
        return false; // notify method not found
    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) == 0) {
            // should call
            return NULL;
        }
    }
    
    //return new_error_object("method '%s' not found in type '%s'", name, type->name);
    return NULL;
}

object *object_to_string(object *obj) {
    if (obj->_type->stringifier != NULL)
        return obj->_type->stringifier(obj);
    return NULL; // or some default?
}

bool objects_are_equal(object *a, object *b) {
    if (a->_type != b->_type)
        return false;
    if (a->_type->equality_checker != NULL)
        return a->_type->equality_checker(a, b);
    return a == b;
}

int object_compare(object *a, object *b) {
    if (a->_type != b->_type)
        return false;
    if (a->_type->comparer != NULL)
        return a->_type->comparer(a, b);
    return -1;
}

unsigned object_hash(object *obj) {
    if (obj->_type->hasher != NULL)
        return obj->_type->hasher(obj);
    return (unsigned)(long)obj;
}

object *object_get_iterator(object *obj) { // create & reset iterator to before first
    if (obj->_type->iterator_factory != NULL)
        return obj->_type->iterator_factory(obj);
    return NULL;
}

object *object_iterator_next(object *obj) { // advance and get next, or return error
    if (obj->_type->iterator_next_implementation != NULL)
        return obj->_type->iterator_next_implementation(obj);
    return NULL;
}

object *object_call(object *obj, object *args, object *named_args) {
    // here is the hard part!
    if (obj->_type->call_handler != NULL)
        return obj->_type->call_handler(obj, args, named_args);
    return NULL;
}
