#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant.h"
#include "../testing.h"


void variant_self_diagnostics(bool verbose) {

    variant *v = new_str_variant("15");
    assert(variant_is_str(v));
    assert(variants_are_equal(v, new_str_variant("15")));
    assert(!variants_are_equal(v, new_int_variant(15))); // no auto conversion
    assert(!variants_are_equal(v, new_float_variant(15.0)));
    assert(variant_as_int(v) == 15); // yes, forced conversion
    assert(variant_as_float(v) == 15.0);

    assert(strcmp(variant_as_str(new_null_variant()), "(null)") == 0);
    assert(strcmp(variant_as_str(new_str_variant("123")), "123") == 0);
    assert(strcmp(variant_as_str(new_int_variant(123)), "123") == 0);
    assert(strcmp(variant_as_str(new_float_variant(3.14)), "3.140000") == 0);
    assert(strcmp(variant_as_str(new_bool_variant(true)), "true") == 0);

    assert(variant_as_bool(new_str_variant("")) == false);
    assert(variant_as_bool(new_str_variant("0")) == false);
    assert(variant_as_bool(new_str_variant("1")) == true);
    assert(variant_as_bool(new_str_variant("true")) == true);
    assert(variant_as_bool(new_str_variant("false")) == false);
    assert(variant_as_bool(new_int_variant(0)) == false);
    assert(variant_as_bool(new_int_variant(1)) == true);
}
