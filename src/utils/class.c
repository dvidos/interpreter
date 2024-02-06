#include <stdlib.h>
#include <string.h>
#include "class.h"


bool classes_are_equal(class *a, class *b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    if (a == b) return true;

    if (strcmp(a->type_name, b->type_name) != 0)
        return false;

    return true;
}
