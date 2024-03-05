#ifndef _VARIANTS_FUNCS_H
#define _VARIANTS_FUNCS_H

#include "_module.h"
#include "variant_base.h"
#include "variant_type.h"

// predefined reusable read only instances
extern variant *true_instance;
extern variant *false_instance;
extern variant *void_instance;
extern variant *zero_instance;
extern variant *one_instance;

void initialize_variants();




// call this to create a new instance
variant *variant_create(variant_type *type, variant *args, variant *named_args);

// references count. if refs down to zero, variant is destroyed too.
void variant_add_ref(variant *obj);
void variant_drop_ref(variant *obj);

// type checks
bool variant_is(variant *obj, variant_type *type);          // type or a subtype of it
bool variant_is_exactly(variant *obj, variant_type *type);

// call these to manipulate properties on an variant
bool     variant_has_attr(variant *obj, const char *name);
variant *variant_get_attr(variant *obj, const char *name);
variant *variant_set_attr(variant *obj, const char *name, variant *value);

// call these to manipulate methods on an variant
bool     variant_has_method(variant *obj, const char *name);
variant *variant_call_method(variant *obj, const char *name, variant *args, variant *named_args);

// a few utilitiy methods without knowing the variant type
variant  *variant_to_string(variant *obj);
bool      variants_new_are_equal(variant *a, variant *b);
int       variant_compare(variant *a, variant *b);
unsigned  variant_hash(variant *obj);
variant  *variant_clone(variant *obj);
variant  *variant_get_iterator(variant *obj); // create & reset iterator to before first
variant  *variant_iterator_next(variant *obj); // advance and get next, or return error
variant  *variant_call(variant *obj, variant *args, variant *named_args);




#endif
