#include <string.h>
#include "variant_funcs.h"
#include "../../utils/error.h"



variant *void_instance;
variant *true_instance;
variant *false_instance;
variant *zero_instance;
variant *one_instance;
variant *iteration_finished_exception_instance;


void initialize_variants() {
    // the following cannot be initialized statically
    type_of_types->_type = type_of_types;

    void_type->_type = type_of_types;
    int_type->_type = type_of_types;
    str_type->_type = type_of_types;
    bool_type->_type = type_of_types;
    float_type->_type = type_of_types;
    exception_type->_type = type_of_types;

    void_instance = new_void_variant();
    true_instance = new_bool_variant(true);
    false_instance = new_bool_variant(false);
    zero_instance = new_int_variant(0);
    one_instance = new_int_variant(1);
    iteration_finished_exception_instance = new_exception_variant("(iteration finished)");

    void_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    true_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    false_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    zero_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    one_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
    iteration_finished_exception_instance->_references_count = VARIANT_STATICALLY_ALLOCATED;
}

variant *variant_create(variant_type *type, variant *args, variant *named_args) {
    if (type == NULL)
        return NULL;
    
    variant *p = malloc(type->instance_size);
    memset(p, 0, type->instance_size);
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

void variant_inc_ref(variant *obj) {
    if (obj == NULL || obj->_type == NULL)
        return;
    if (obj->_references_count == VARIANT_STATICALLY_ALLOCATED)
        return;
    
    obj->_references_count++;
}

void variant_drop_ref(variant *obj) {
//    if (obj == NULL || obj->_type == NULL)
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

bool variant_instance_of(variant *obj, variant_type *type) {
    if (obj == NULL || obj->_type == NULL)
        return false;
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
    if (obj == NULL || obj->_type == NULL)
        return false;
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

execution_outcome variant_get_attr_value(variant *obj, const char *name) {
    variant_type *type = obj->_type;
    if (type->attributes == NULL)
        return exception_outcome(new_exception_variant("attribute '%s' not found in type '%s'", name, type->name));

    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;
        
        type_attrib_definition *def = &type->attributes[i];
        if (def->getter != NULL) {
            return def->getter(obj, name);

        } else if (def->tat_flags & TAT_VARIANT_PTR) {
            variant *obj_ptr_ = (variant *)(((char *)obj) + def->offset);
            variant_inc_ref(obj_ptr_); // the one returned
            return ok_outcome(obj_ptr_);

        } else if (def->tat_flags & TAT_INT) {
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            return ok_outcome(new_int_variant(*int_ptr));

        } else if (def->tat_flags & TAT_BOOL) {
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            return ok_outcome(new_bool_variant(*bool_ptr));

        } else if (def->tat_flags & TAT_CONST_CHAR_PTR) {
            const char *char_ptr = (((const char *)obj) + def->offset);
            return ok_outcome(new_str_variant(char_ptr));

        } else {
            return failed_outcome("attribute '%s' not supported type '%d'", name, def->tat_flags);
        }
    }

    return exception_outcome(new_exception_variant("attribute '%s' not found in type '%s'", name, type->name));
}

execution_outcome variant_set_attr_value(variant *obj, const char *name, variant *value) {
    variant_type *type = obj->_type;
    if (type->attributes == NULL)
        return exception_outcome(new_exception_variant("attribute '%s' not found in type '%s'", name, type->name));
        
    for (int i = 0; type->attributes[i].name != NULL; i++) {
        if (strcmp(type->attributes[i].name, name) != 0)
            continue;

        type_attrib_definition *def = &type->attributes[i];
        if (def->tat_flags & TAT_READ_ONLY)
            return exception_outcome(new_exception_variant("attribute '%s' is read only in type '%s'", name, type->name));

        if (def->setter != NULL) {
            return def->setter(obj, name, value);

        } else if (def->tat_flags & TAT_VARIANT_PTR) {
            variant **obj_ptr_ptr = (variant **)(((char *)obj) + def->offset);
            *obj_ptr_ptr = value;
            variant_inc_ref(value);
            return ok_outcome(NULL);

        } else if (def->tat_flags & TAT_INT) {
            if (!variant_instance_of(value, int_type))
                return exception_outcome(new_exception_variant("cannot assign type '%s' to int attribute %s.%s", value->_type->name, obj->_type->name, name));
            int *int_ptr = (int *)(((char *)obj) + def->offset);
            *int_ptr = int_variant_as_int(value);
            return ok_outcome(NULL);

        } else if (def->tat_flags & TAT_BOOL) {
            if (!variant_instance_of(value, bool_type))
                return exception_outcome(new_exception_variant("cannot assign type '%s' to bool attribute %s.%s", value->_type->name, obj->_type->name, name));
            bool *bool_ptr = (bool *)(((char *)obj) + def->offset);
            *bool_ptr = bool_variant_as_bool(value);
            return ok_outcome(NULL);

        } else if (def->tat_flags & TAT_CONST_CHAR_PTR) {
            if (!variant_instance_of(value, str_type))
                return exception_outcome(new_exception_variant("cannot assign type '%s' to str attribute %s.%s", value->_type->name, obj->_type->name, name));
            const char **char_ptr_ptr = (const char **)(((char *)obj) + def->offset);
            *char_ptr_ptr = str_variant_as_str(value); // we should copy maybe instead of refering to?
            return ok_outcome(NULL);

        } else {
            return exception_outcome(new_exception_variant("attribute '%s' has not supported type '%d'", def->tat_flags));
        }
    }
    return exception_outcome(new_exception_variant("attribute '%s' not found in type '%s'", name, type->name));
}

bool variant_has_method(variant *obj, const char *name) {
    variant_type *type = obj->_type;
    if (type->methods == NULL)
        return false;
    
    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) == 0) {
            return true;
        }
    }

    return false;
}

execution_outcome variant_call_method(variant *obj, const char *name, variant *args_list, variant *named_args) {

    variant_type *type = obj->_type;
    if (type->methods == NULL)
        return exception_outcome(new_exception_variant("method '%s()' not found in type '%s'", name, type->name));

    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) != 0)
            continue;
        
        type_method_definition *tmd = &type->methods[i];
        return tmd->handler(obj, args_list, named_args);
    }
    
    return exception_outcome(new_exception_variant("method '%s()' not found in type '%s'", name, type->name));
}

execution_outcome variant_get_bound_method(variant *obj, const char *name) {

    variant_type *type = obj->_type;
    if (type->methods == NULL)
        return exception_outcome(new_exception_variant("method '%s()' not found in type '%s'", name, type->name));

    for (int i = 0; type->methods[i].name != NULL; i++) {
        if (strcmp(type->methods[i].name, name) != 0)
            continue;
        
        // we must make a class that is callable by design!!!!!
        // same thing could be used for an expression function that has captured variables.
        variant *bound_method = new_callable_variant(
            NULL, // TODO: fix this.
            obj
        );
        // but we need the callable, no?
        return ok_outcome(bound_method);
    }
    
    return exception_outcome(new_exception_variant("method '%s()' not found in type '%s'", name, type->name));
}

variant *variant_to_string(variant *obj) {
    if (obj == NULL)
        return new_str_variant("(null)");
    
    if (obj->_type->stringifier == NULL)
        return new_str_variant("(%s @ 0x%p)", obj->_type->name, obj);
    
    return obj->_type->stringifier(obj);
}

bool variants_are_equal(variant *a, variant *b) {
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

execution_outcome variant_iterator_next(variant *obj) { // advance and get next, or return error
    if (obj->_type->iterator_next_implementation == NULL)
        return exception_outcome(new_exception_variant("Type '%s' is not iterable", obj->_type->name));

    return obj->_type->iterator_next_implementation(obj);
}

execution_outcome variant_call(variant *obj, variant *args_list, variant *named_args) {
    if (obj == NULL || obj->_type == NULL)
        return failed_outcome("Expecting variant with a type, got null");

    if (obj->_type->call_handler == NULL)
        return exception_outcome(new_exception_variant("Type '%s' is not callable", obj->_type->name));

    return obj->_type->call_handler(obj, args_list, named_args);
}

execution_outcome variant_get_element(variant *obj, variant *index) {
    if (obj->_type->get_element == NULL)
        return exception_outcome(new_exception_variant("Type '%s' does not support getting element by index", obj->_type->name));

    return obj->_type->get_element(obj, index);
}

execution_outcome variant_set_element(variant *obj, variant *index, variant *value) {
    if (obj->_type->set_element == NULL)
        return exception_outcome(new_exception_variant("Type '%s' does not support setting element by index", obj->_type->name));
    
    return obj->_type->set_element(obj, index, value);
}
