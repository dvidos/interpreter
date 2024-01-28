#include <stdlib.h>
#include <string.h>
#include "contained_item.h"


bool contained_item_info_are_equal(contained_item *a, contained_item *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    if (a == b) return true;

    if (strcmp(a->type_name, b->type_name) != 0)
        return false;

    return true;
}
