#include "../../utils/containers/dict.h"
#include "../../utils/variant.h"

struct exec_context {
    dict *callables;
    dict *global_variables;
    variant_type expected_returned_value_type;
    stream *input;
    stream *output;
};

