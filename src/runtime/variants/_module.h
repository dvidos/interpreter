#ifndef _VARIANTS_MODULE_H
#define _VARIANTS_MODULE_H


// forward declarations
typedef struct variant variant;
typedef struct variant_type variant_type;

#include "variant_base.h" 
#include "variant_type.h" 
#include "variant_funcs.h"
#include "variant_original.h"

void variant_self_diagnostics(bool verbose);


#endif
