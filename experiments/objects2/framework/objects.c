#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "objects.h"

/*  Despite their name 'object', the structures below represent 
    the _classes_ of the instances. They allow instances to be created using 'new'

    Each object contains a copy of the base `object` contents, 
    therefore can be used or cast to a base object.

    Each object has a pointer named 'type', that points to a type_object that
    describes this class, and contains the methods of the instances.

    The type_object's type points to a single instance of the 'type' instance.

    For more code than you can read in a day...
    see https://github.com/python/cpython/blob/main/Include/object.h#L554-L588 
    see https://docs.python.org/3/c-api/structures.html#base-object-types-and-macros

    Essentially, the objects and methods in this file, should be direct
    implementation of the expressions in the language and the AST tree.
    Example `is_true(object *)` should be used to evaluate a value as bool.
*/



// first, the poorer man's error subsystem
static char error_message[256] = {0};
void set_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(error_message, sizeof(error_message), fmt, args);
    va_end(args);
}
void clear_error() {
    error_message[0] = '\0';
}
bool is_error() {
    return error_message[0] != 0;
}
const char *get_error() {
    return error_message;
}



object *new_typed_instance(type_object *type, object *args, object *named_args) {
    object *p = mem_alloc(type->instance_size);
    p->type = type;
    p->references_count = 1; // the one we are going to return
    if (type->initializer != NULL) {
        type->initializer(p, args, named_args);
    }
    return p;
}

object *new_named_instance(const char *type_name, object *args, object *named_args) {
    type_object *type = objects_get_named_type(type_name);
    if (type == NULL) {
        set_error("type '%s' not found", type_name);
        return NULL;
    }
    return new_typed_instance(type, args, named_args);
}

void object_destruct_and_free(object *obj) {
    if (obj->type->destructor)
        obj->type->destructor(obj);
    mem_free(&obj);
}

void object_incr_ref_count(object *obj) {
    obj->references_count++;
}

void object_decr_ref_count(object *obj) {
    if (obj == NULL)
        return;
    if (obj->references_count == OBJECT_STATICALLY_ALLOCATED)
        return;
    
    obj->references_count--;
    if (obj->references_count > 0)
        return;
    
    // no more references to this object
    // we need to finalize it and free it up
    object_destruct_and_free(obj);
}

bool object_is(object *obj, type_object *type) {
    type_object *t = obj->type;
    int levels = 0; // avoid infinite loops
    while (t != NULL && levels++ < 100) {
        if (t == type)
            return true;
        t = t->base_type;
    }
    return false;
}

bool object_is_exactly(object *obj, type_object *type) {
    return obj->type == type;
}





bool object_has_attr(object *obj, const char *name) {
    type_object *t = obj->type;
    if (t->attributes == NULL) return false;
    for (int i = 0; t->attributes[i]->name != NULL; i++) {
        if (strcmp(t->attributes[i]->name, name) == 0) {
            return true;
        }
    }
    return false;
}

object *object_get_attr(object *obj, const char *name) {
    type_object *type = obj->type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i]->name != NULL; i++) {
        if (strcmp(type->attributes[i]->name, name) != 0)
            continue;
        
        type_attrib_definition *def = type->attributes[i];
        if (def->getter != NULL) {
            return def->getter(obj, name);
        } else {
            // read with internal generic getter
        }
    }

    set_error("attribute '%s' not found in type '%s'", name, type->name);
    return NULL;
}

object *object_set_attr(object *obj, const char *name, object *value) {
    type_object *type = obj->type;
    if (type->attributes == NULL) return false;
    for (int i = 0; type->attributes[i]->name != NULL; i++) {
        if (strcmp(type->attributes[i]->name, name) != 0)
            continue;

        type_attrib_definition *def = type->attributes[i];
        if (def->tat_flags & TAT_READ_ONLY) {
            set_error("attribute '%s' is read only in type '%s'", name, type->name);
            return NULL;
        }

        if (def->setter != NULL) {
            return def->setter(obj, name, value); // error checking
        } else {
            // set directly using internal logic. (e.g. integers)
            ... how to get actual value from the objectified type?
            see https://docs.python.org/3/c-api/long.html#c.PyLong_AsLong
            using PyLong_AsLong() or in our case: int_object_as_int()...
            also we have new_long_object_from_long()
        }
    }
    set_error("attribute '%s' not found in type '%s'", name, type->name);
    return NULL;
}

bool object_has_method(object *obj, const char *name) {
    type_object *type = obj->type;
    if (type->methods == NULL) return false;
    for (int i = 0; type->methods[i]->name != NULL; i++) {
        if (strcmp(type->methods[i]->name, name) == 0) {
            return true;
        }
    }
    return false;
}

object *object_call_method(object *obj, const char *name, object *args, object *named_args) {
    type_object *type = obj->type;
    if (type->methods == NULL)
        return false; // notify method not found
    for (int i = 0; type->methods[i]->name != NULL; i++) {
        if (strcmp(type->methods[i]->name, name) == 0) {
            // should call
            return NULL;
        }
    }
    return new_error_object("method '%s' not found in type '%s'", name, type->name);
}

object *object_to_string(object *obj) {
    if (obj->type->stringifier != NULL)
        return obj->type->stringifier(obj);
    return NULL; // or some default?
}

bool objects_are_equal(object *a, object *b) {
    if (a->type != b->type)
        return false;
    if (a->type->equality_checker != NULL)
        return a->type->equality_checker(a, b);
    return a == b;
}

int object_compare(object *a, object *b) {
    if (a->type != b->type)
        return false;
    if (a->type->comparer != NULL)
        return a->type->comparer(a, b);
    return -1;
}

unsigned object_hash(object *obj) {
    if (obj->type->hasher != NULL)
        return obj->type->hasher;
    return (unsigned)(long)obj;
}

object *object_get_iterator(object *obj) { // create & reset iterator to before first
    if (obj->type->iterator_factory != NULL)
        return obj->type->iterator_factory(obj);
    return NULL;
}

object *object_iterator_next(object *obj) { // advance and get next, or return error
    if (obj->type->iterator_next_implementation != NULL)
        return obj->type->iterator_next_implementation;
    return NULL;
}

object *object_call(object *obj, object *args, object *named_args) {
    // here is the hard part!
    if (obj->type->call_implementation != NULL)
        return obj->type->call_implementation(obj, args, named_args);
    return NULL;
}

