#ifndef _EXECUTION_OUTCOME_H
#define _EXECUTION_OUTCOME_H

#include "../utils/data_types/variant.h"

// any function can instantiate and return an exception
// so we need an alternative way to signal a "thrown" exception
typedef struct execution_outcome {
    // succesful execution
    variant *result;

    // exception thrown because of the script (e.g. division by zero)
    bool exception_thrown;
    variant *exception;

    // failure due to our code (e.g. out of memory)
    bool failed;
    const char *failure_message;
} execution_outcome;


execution_outcome ok_outcome(variant *result);
execution_outcome exception_outcome(variant *exception);
execution_outcome failed_outcome(const char *fmt, ...);


#endif
