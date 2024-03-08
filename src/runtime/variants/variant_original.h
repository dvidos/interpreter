#ifndef _VARIANT_H
#define _VARIANT_H

#include <stdbool.h>
#include "../../containers/_module.h"

typedef struct callable callable;
typedef struct variant variant;


const char *deprecated_variant_as_const_char(variant *v);

extern item_info *variant_item_info;



#endif
