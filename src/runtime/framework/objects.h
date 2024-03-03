#ifndef _OBJECTS_H
#define _OBJECTS_H

/*  Despite their name 'object', the structures represent the _classes_ 
    of the instances. They allow instances to be created using 'new'

    Each object contains a copy of the base `object` contents, 
    therefore can be used or cast to a base object.

    Each object has a pointer named 'type', that points to a object_type that
    describes this class, and contains the methods of the instances.

    The object_type's type points to a single instance of the 'type' instance.

    For more code than you can read in a day...
    see https://github.com/python/cpython/blob/main/Include/object.h#L554-L588 
    see https://docs.python.org/3/c-api/structures.html#base-object-types-and-macros

    Essentially, the objects and methods in this file, should be direct
    implementation of the expressions in the language and the AST tree.
    Example `is_true(object *)` should be used to evaluate a value as bool.
*/

#include <stdlib.h> // for NULL
#include <stddef.h> // for offsetof()
#include <stdbool.h> // for bool


// forward declarations
typedef struct object object;
typedef struct object_type object_type;


#include "../../utils/mem.h"
#include "../../utils/error.h"
#include "base_object.h" 
#include "object_type.h" 
#include "object_funcs.h"



#endif
