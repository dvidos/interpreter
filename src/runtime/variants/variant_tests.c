#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant_item_info.h"
#include "../../utils/testing.h"
#include "_internal.h"

void variant_self_diagnostics(bool verbose) {

    variant *v = new_str_variant("15");
    assert(strcmp(str_variant_as_str(v), "15") == 0);
    
    v = new_int_variant(15);
    assert(strcmp(str_variant_as_str(variant_to_string(v)), "15") == 0);

    // assert(variant_instance_of(v, str_type));
    // assert(variants_are_equal(v, new_str_variant("15")));
    // assert(!variants_are_equal(v, new_int_variant(15))); // no auto conversion
    // assert(!variants_are_equal(v, new_float_variant(15.0)));
    // assert(int_variant_as_int(v) == 15); // yes, forced conversion
    // assert(float_variant_as_float(v) == 15.0);

    // assert(strcmp(deprecated_variant_as_const_char(void_instance), "(void)") == 0);
    // assert(strcmp(deprecated_variant_as_const_char(new_str_variant("123")), "123") == 0);
    // assert(strcmp(deprecated_variant_as_const_char(new_int_variant(123)), "123") == 0);
    // assert(strcmp(deprecated_variant_as_const_char(new_float_variant(3.14)), "3.140000") == 0);
    // assert(strcmp(deprecated_variant_as_const_char(new_bool_variant(true)), "true") == 0);

    // we have to setup basic conversion functions:
    // str(), int(), float(), bool().
    
    // assert(variant_as_bool(new_str_variant("")) == false);
    // assert(variant_as_bool(new_str_variant("0")) == false);
    // assert(variant_as_bool(new_str_variant("1")) == true);
    // assert(variant_as_bool(new_str_variant("true")) == true);
    // assert(variant_as_bool(new_str_variant("false")) == false);
    // assert(variant_as_bool(new_int_variant(0)) == false);
    // assert(variant_as_bool(new_int_variant(1)) == true);
}
