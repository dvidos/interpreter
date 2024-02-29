#ifndef _EXECUTION_OUTCOME_H
#define _EXECUTION_OUTCOME_H

#include "../utils/data_types/variant.h"

// distinguish between the script failing and our engine failing
typedef struct execution_outcome {
    variant *successful;
    bool exception_thrown;
    variant *exception;
} execution_outcome;

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(execution_outcome);
#define failed_execution_outcome(inner, fmt, ...)  __failed_execution_outcome(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

execution_outcome *new_execution_outcome_successful(variant *successful);
execution_outcome *new_execution_outcome_exception(variant *exception);


#endif
