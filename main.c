#include "utils/testing.h"
#include "utils/variant.h"
#include "code/lexer/tokenization_tests.h"
#include "code/parser/parser_tests.h"
#include "code/interpreter_tests.h"
#include "code/interpreter.h"


/*
    The core of the functionality is the "interpret_and_execute()" function.
    It would give the ability to programs to evaluate strings,
    like complex business conditions etc.

    To do this we need a whole ecosystem for these:
    - values (strings, numbers, booleans, lists, dictionaries)
    - parser of expressions with precedence, into an abstract syntax tree
    - evaluation of the syntax tree, to find the result

    We would love to include function calls in the expression parser,
    something like "if(left(a, 1) == '0', 'number', 'letter')"
    Maybe the code body, arguments and returned value can become a function.

    interpret_and_execute("a + b * 2", {a=1, b=2})
*/

bool run_self_diagnostics() {

    variant_self_diagnostics();
    tokenizer_self_diagnostics();
    parser_self_diagnostics(false);
    interpreter_self_diagnostics();

    return testing_outcome(true);
}


int main() {
    initialize_interpreter();

    if (!run_self_diagnostics())
        return 1;
    
    // actual functionality here
    return 0;
}
