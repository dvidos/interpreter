#ifndef _PAIR_H
#define _PAIR_H

#include "../failable.h"
#include "../iterator.h"
#include "contained_item.h"


typedef struct pair pair;

pair *new_pair(contained_item *left_item, void *left, contained_item *right_item, void *right);

void *pair_get_left(pair *p);
void *pair_get_right(pair *p);

bool pairs_are_equal(pair *a, pair *b);
const void pair_describe(pair *p, const char *separator, str_builder *sb);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(pair);
#define failed_pair(inner, fmt, ...)  __failed_pair(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

extern contained_item *containing_pairs;

#endif
