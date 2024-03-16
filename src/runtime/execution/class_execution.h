#ifndef _CLASS_EXECUTION_H
#define _CLASS_EXECUTION_H

#include "../variants/_module.h"
#include "../../entities/statement.h"


// from a class statement, create a variant type
variant_type *class_statement_create_variant_type(statement *stmt);


#endif
