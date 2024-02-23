#ifndef _OBJECTS_FUNCS_H
#define _OBJECTS_FUNCS_H


// call this to create a new instance
object *object_create(object_type *type, object *args, object *named_args);

// references count. if refs down to zero, object is destroyed too.
void object_add_ref(object *obj);
void object_drop_ref(object *obj);

// type checks
bool object_is(object *obj, object_type *type);          // type or a subtype of it
bool object_is_exactly(object *obj, object_type *type);  // type or a subtype of it


// call these to manipulate properties on an object
bool    object_has_attr(object *obj, const char *name);
object *object_get_attr(object *obj, const char *name);
object *object_set_attr(object *obj, const char *name, object *value);

// call these to manipulate methods on an object
bool object_has_method(object *obj, const char *name);
object *object_call_method(object *obj, const char *name, object *args, object *named_args);

// a few utilitiy methods without knowing the object type
object  *object_to_string(object *obj);
bool     objects_are_equal(object *a, object *b);
int      object_compare(object *a, object *b);
unsigned object_hash(object *obj);
object  *object_clone(object *obj);
object  *object_get_iterator(object *obj); // create & reset iterator to before first
object  *object_iterator_next(object *obj); // advance and get next, or return error
object  *object_call(object *obj, object *args, object *named_args);




#endif
