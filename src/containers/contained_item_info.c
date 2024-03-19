#include <stdlib.h>
#include <string.h>
#include "contained_item_info.h"


bool item_infos_are_equal(contained_item_info *a, contained_item_info *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    if (a == b) return true;

    if (strcmp(a->type_name, b->type_name) != 0)
        return false;

    return true;
}
