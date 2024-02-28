#ifndef _EXECUTION_RESULT_H
#define _EXECUTION_RESULT_H

#include "../utils/data_types/variant.h"

// distinguish between the script failing and our engine failing
typedef struct execution_result {
    variant *result;
    bool exception_thrown;
    variant *exception;
} execution_result;

STRONGLY_TYPED_FAILABLE_PTR_INLINE(execution_result);
#define failed_execution_result(inner, fmt, ...)  __failed_execution_result(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

execution_result *new_execution_result(variant *result);
execution_result *new_execution_result_exception(variant *exception);



#endif
