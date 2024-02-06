#include <stdlib.h>
#include <string.h>
#include "containers_tests.h"
#include "list.h"
#include "dict.h"
#include "queue.h"
#include "stack.h"
#include "../testing.h"

#include "../data_types/variant.h"

bool containers_self_diagnostics(bool verbose) {
    str_builder *sb = new_str_builder();

    queue *q = new_queue(containing_variants);
    assert(queue_empty(q));
    assert(queue_length(q) == 0);
    queue_put(q, new_str_variant("a"));
    queue_put(q, new_str_variant("b"));
    queue_put(q, new_str_variant("c"));
    assert(!queue_empty(q));
    assert(queue_length(q) == 3);

    str_builder_clear(sb);
    queue_describe(q, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "a|b|c") == 0);
    
    assert(strcmp(variant_as_str(queue_get(q)), "a") == 0);
    assert(strcmp(variant_as_str(queue_get(q)), "b") == 0);
    assert(strcmp(variant_as_str(queue_get(q)), "c") == 0);
    assert(queue_empty(q));
    assert(queue_length(q) == 0);

    return testing_outcome(false);
}
