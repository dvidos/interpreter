#include <stdlib.h>
#include <string.h>
#include "containers_tests.h"
#include "list.h"
#include "dict.h"
#include "queue.h"
#include "stack.h"
#include "../utils/testing.h"

#include "../runtime/variants/_module.h"



static void test_list() {
    str_builder *sb = new_str_builder();

    list *l = new_list(variant_class);
    assert(list_empty(l));
    assert(list_length(l) == 0);
    list_add(l, new_str_variant("a"));
    list_add(l, new_str_variant("b"));
    list_add(l, new_str_variant("c"));
    assert(!list_empty(l));
    assert(list_length(l) == 3);
    assert(strcmp(variant_as_str(list_get(l, 2)), "c") == 0);

    str_builder_clear(sb);
    list_describe(l, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "a|b|c") == 0);

    list_insert(l, 0, new_str_variant("h"));
    str_builder_clear(sb);
    list_describe(l, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "h|a|b|c") == 0);

    list_insert(l, 2, new_str_variant("v"));
    str_builder_clear(sb);
    list_describe(l, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "h|a|v|b|c") == 0);

    list_remove(l, 0);
    list_remove(l, 2);
    str_builder_clear(sb);
    list_describe(l, "|", sb);
    assert(strcmp(str_builder_charptr(sb), "a|v|c") == 0);
}

static void test_dict() {

}

static void test_stack() {
    str_builder *sb = new_str_builder();

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

static void test_queue() {
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
}

void containers_self_diagnostics(bool verbose) {
    test_list();
    test_dict();
    test_stack();
    test_queue();
}
