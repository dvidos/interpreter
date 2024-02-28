#include <stdlib.h>
#include <stdbool.h>
#include "execution_result.h"


execution_result *new_execution_result(variant *result) {
    execution_result *r = malloc(sizeof(execution_result));
    r->result = result;
    r->exception_thrown = false;
    r->exception = NULL;
    return r;
}

execution_result *new_execution_result_exception(variant *exception) {
    execution_result *r = malloc(sizeof(execution_result));
    r->result = NULL;
    r->exception_thrown = true;
    r->exception = exception;
    return r;
}

