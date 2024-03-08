#ifndef _VARIANT_INTERNAL_H
#define _VARIANT_INTERNAL_H


// forward declarations
typedef struct variant variant;
typedef struct variant_type variant_type;

#include <stdlib.h> // for NULL (but gives us malloc() too)
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool

#include "../../utils/mem.h"
#include "../../utils/error.h"

#include "variant_base.h" 
#include "variant_type.h" 
#include "variant_funcs.h"
#include "variant_item_info.h"


#include "void_variant.h"
#include "str_variant.h"
#include "int_variant.h"
#include "bool_variant.h"
#include "float_variant.h"
#include "exception_variant.h"
#include "list_variant.h"
#include "dict_variant.h"
#include "callable_variant.h"


#endif
