#ifndef _PAIR_H
#define _PAIR_H

#include "../failable.h"
#include "../iterator.h"
#include "../class.h"


typedef struct pair pair;

pair *new_pair(class *left_item_class, void *left, class *right_item_class, void *right);

void *pair_get_left(pair *p);
void *pair_get_right(pair *p);

bool pairs_are_equal(pair *a, pair *b);
const void pair_describe(pair *p, const char *separator, str_builder *sb);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(pair);
#define failed_pair(inner, fmt, ...)  __failed_pair(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

extern class *pair_class;

#endif
