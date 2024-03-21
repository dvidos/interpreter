#include <stdlib.h>
#include <string.h>
#include "containers_tests.h"
#include "list.h"
#include "dict.h"
#include "queue.h"
#include "stack.h"
#include "../utils/testing.h"

#include "../runtime/variants/_variants.h"



static void test_list() {
    str *s = new_str();

    list *l = new_list(variant_item_info);
    assert(list_empty(l));
    assert(list_length(l) == 0);
    list_add(l, new_str_variant("a"));
    list_add(l, new_str_variant("b"));
    list_add(l, new_str_variant("c"));
    assert(!list_empty(l));
    assert(list_length(l) == 3);
    assert(strcmp(str_variant_as_str(variant_to_string(list_get(l, 2))), "c") == 0);

    str_clear(s);
    list_describe(l, "|", s);
    assert(strcmp(str_cstr(s), "a|b|c") == 0);

    list_insert(l, 0, new_str_variant("h"));
    str_clear(s);
    list_describe(l, "|", s);
    assert(strcmp(str_cstr(s), "h|a|b|c") == 0);

    list_insert(l, 2, new_str_variant("v"));
    str_clear(s);
    list_describe(l, "|", s);
    assert(strcmp(str_cstr(s), "h|a|v|b|c") == 0);

    list_remove(l, 0);
    list_remove(l, 2);
    str_clear(s);
    list_describe(l, "|", s);
    assert(strcmp(str_cstr(s), "a|v|c") == 0);
}

static void test_dict() {

}

static void test_stack() {
    str *s = new_str();

    stack *stk = new_stack(variant_item_info);
    assert(stack_empty(stk));
    assert(stack_length(stk) == 0);
    stack_push(stk, new_str_variant("a"));
    stack_push(stk, new_str_variant("b"));
    stack_push(stk, new_str_variant("c"));
    assert(!stack_empty(stk));
    assert(stack_length(stk) == 3);
    assert(strcmp(str_variant_as_str(variant_to_string(stack_peek(stk))), "c") == 0);

    str_clear(s);
    stack_describe(stk, "|", s);
    assert(strcmp(str_cstr(s), "c|b|a") == 0);
    
    assert(strcmp(str_variant_as_str(variant_to_string(stack_pop(stk))), "c") == 0);
    assert(strcmp(str_variant_as_str(variant_to_string(stack_pop(stk))), "b") == 0);
    assert(strcmp(str_variant_as_str(variant_to_string(stack_pop(stk))), "a") == 0);
    assert(stack_empty(stk));
    assert(stack_length(stk) == 0);
}

static void test_queue() {
    str *s = new_str();

    queue *q = new_queue(variant_item_info);
    assert(queue_empty(q));
    assert(queue_length(q) == 0);
    queue_put(q, new_str_variant("a"));
    queue_put(q, new_str_variant("b"));
    queue_put(q, new_str_variant("c"));
    assert(!queue_empty(q));
    assert(queue_length(q) == 3);

    str_clear(s);
    queue_describe(q, "|", s);
    assert(strcmp(str_cstr(s), "a|b|c") == 0);
    
    assert(strcmp(str_variant_as_str(variant_to_string(queue_get(q))), "a") == 0);
    assert(strcmp(str_variant_as_str(variant_to_string(queue_get(q))), "b") == 0);
    assert(strcmp(str_variant_as_str(variant_to_string(queue_get(q))), "c") == 0);
    assert(queue_empty(q));
    assert(queue_length(q) == 0);
}

void containers_self_diagnostics(bool verbose) {
    test_list();
    test_dict();
    test_stack();
    test_queue();
}
