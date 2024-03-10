#ifndef _VARIANTS_FUNCS_H
#define _VARIANTS_FUNCS_H

#include "_internal.h"


// predefined reusable read only instances
extern variant *true_instance;
extern variant *false_instance;
extern variant *void_instance;
extern variant *zero_instance;
extern variant *one_instance;
extern variant *iteration_finished_exception_instance;

void initialize_variants();




// call this to create a new instance
variant *variant_create(variant_type *type, variant *args, variant *named_args);

// references count. if refs down to zero, variant is destroyed too.
void variant_inc_ref(variant *obj);
void variant_drop_ref(variant *obj);

// type checks
bool variant_instance_of(variant *obj, variant_type *type);          // type or a subtype of it
bool variant_is_exactly(variant *obj, variant_type *type);

// call these to manipulate properties on an variant
bool              variant_has_attr(variant *obj, const char *name);
execution_outcome variant_get_attr_value(variant *obj, const char *name);
execution_outcome variant_set_attr_value(variant *obj, const char *name, variant *value);

// call these to manipulate methods on an variant
bool              variant_has_method(variant *obj, const char *name);
execution_outcome variant_call_method(variant *obj, const char *name, list *args_list, dict *named_args, exec_context *ctx);
execution_outcome variant_get_bound_method(variant *obj, const char *name);

// a few utilitiy methods without knowing the variant type
variant *         variant_to_string(variant *obj);
bool              variants_are_equal(variant *a, variant *b);
int               variant_compare(variant *a, variant *b);
unsigned          variant_hash(variant *obj);
variant *         variant_clone(variant *obj);
variant *         variant_get_iterator(variant *obj); // create & reset iterator to before first
execution_outcome variant_iterator_next(variant *obj); // advance and get next, or return error

execution_outcome variant_call(variant *obj, list *args, dict *named_args, exec_context *ctx);
execution_outcome variant_get_element(variant *obj, variant *index);
execution_outcome variant_set_element(variant *obj, variant *index, variant *value);



#endif
