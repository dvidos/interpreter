#include "tests.h"
#include "eval/eval.h"


/*
    The core of the functionality is the "evaluate()" function.
    It would give the ability to programs to evaluate strings,
    like complex business conditions etc.

    To do this we need a whole ecosystem for these:
    - values (strings, numbers, booleans, lists, dictionaries)
    - parser of expressions with precedence, into an abstract syntax tree
    - evaluation of the syntax tree, to find the result

    We would love to include function calls in the expression parser,
    something like "if(left(a, 1) == '0', 'number', 'letter')"
    Maybe the code body, arguments and returned value can become a function.

    evaluate("a + b * 2", {a=1, b=2})
*/

int main() {
    initialize_evaluator();

    if (!run_unit_tests())
        return 1;
    
    // actual functionality here
    return 0;
}


