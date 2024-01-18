#include "../../utils/containers/dict.h"
#include "../../utils/variant.h"
#include "../../utils/str_builder.h"

struct exec_context {
    dict *callables;
    dict *global_variables;
    variant_type expected_returned_value_type;
    str_builder *log_output;
};

