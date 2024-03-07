#ifndef _ITEM_INFO_H
#define _ITEM_INFO_H

#include <stdbool.h>
#include "str_builder.h"

#define ITEM_INFO_MAGIC  0xC1A55DEF

typedef bool (*items_equal_func)(void *pointer_a, void *pointer_b);
typedef void (*describe_item_func)(void *pointer, str_builder *sb);
typedef unsigned long *(*hash_item_func)(void *pointer);

typedef struct item_info {
    unsigned int item_info_magic;
    const char *type_name;
    items_equal_func are_equal;
    describe_item_func  describe;
    hash_item_func      hash;
} item_info;

bool item_infos_are_equal(item_info *a, item_info *b);


#endif
