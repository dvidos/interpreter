#include <stdlib.h>
#include <stdbool.h>
#include "execution_outcome.h"


STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(execution_outcome);


execution_outcome *new_execution_outcome_successful(variant *successful) {
    execution_outcome *o = malloc(sizeof(execution_outcome));
    o->successful = successful;
    o->exception_thrown = false;
    o->exception = NULL;
    return o;
}

execution_outcome *new_execution_outcome_exception(variant *exception) {
    execution_outcome *o = malloc(sizeof(execution_outcome));
    o->successful = NULL;
    o->exception_thrown = true;
    o->exception = exception;
    return o;
}

