#include <stdlib.h>
#include <string.h>
#include "containers_tests.h"
#include "list.h"
#include "dict.h"
#include "queue.h"
#include "stack.h"
#include "../testing.h"

#include "../data_types/variant.h"

void containers_self_diagnostics(bool verbose) {
    str_builder *sb = new_str_builder();

    queue *q = new_queue(variant_class);
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


    stack *s = new_stack(variant_class);
    assert(stack_empty(s));
    assert(stack_length(s) == 0);
    stack_push(s, new_str_variant("a"));
    stack_push(s, new_str_variant("b"));
    stack_push(s, new_str_variant("c"));
    assert(!stack_empty(s));
    assert(stack_length(s) == 3);
    assert(strcmp(variant_as_str(stack_peek(s)), "c") == 0);

    str_builder_clear(sb);
    stack_describe(s, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "c|b|a") == 0);
    
    assert(strcmp(variant_as_str(stack_pop(s)), "c") == 0);
    assert(strcmp(variant_as_str(stack_pop(s)), "b") == 0);
    assert(strcmp(variant_as_str(stack_pop(s)), "a") == 0);
    assert(stack_empty(s));
    assert(stack_length(s) == 0);
}
