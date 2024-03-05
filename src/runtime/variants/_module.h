#ifndef _VARIANTS_MODULE_H
#define _VARIANTS_MODULE_H


// forward declarations
typedef struct variant variant;
typedef struct variant_type variant_type;


#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

#include "../../utils/mem.h"
#include "../../utils/error.h"

#include "variant_base.h" 
#include "variant_type.h" 
#include "variant_funcs.h"
#include "variant_original.h"

void variant_self_diagnostics(bool verbose);



#endif
