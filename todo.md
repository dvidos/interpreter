# todo

* make calling user functions and user methods possible.
* fix / simplify both parsers (expression,statement) to have: accept(), peek(), accepted() etc.
* symbol tables should contain info about variable type or function type, flags etc.
* make function an expression, in order for it to be used as a value and passed around, 
returned from functions etc.
* extract the various "types" (token type, statement type, expression type, value type) on
separate files, to allow better management.


* [maybe] consider removing getter functions and allow direct reading of struct members.
for speed. and simplicity.
* make iterator a two level object, internal is move/validate only, externa supports:
`last`, `curr`, `peek` etc, to aleviate code repetition. Then, since we have the `last` method, fix:
  * the `if(a;b;c;d) {...} case
  * function arguments parsing, so that parser does not need to store last token.
* implement parsing of functions

* make calling of user defined functions.
* find a way to implement objects using dictionaries and the `this` keyword.
* make better iterators, minimal ask from conntainers.
* make streams for execution, stdin/out etc? streams should also work per 
object, e.g. a json object, not only per text line or bytes.
* could implement a `foreach` keyword, to iterate over 

* instead of multiple "switch" statements to handle polymorphic behavior,
convert to function pointers, and implement discrete functions for each case.
This should speed up runtime a ton.

* experiment with idea of declaration & implementation of strongly typed containers
through macros, akin to failables...

## later

Ability to parse EBNF syntax and then parse things on the fly?



# done!

* make parsing of dictionaries `a = {key: value, ...}` and lists `b = (1, 2, ...)`
* make log() echo to stderr, unless suppressed or redirected to a log file.
* try out the idea of `_module.h` files
* fix first assignment to create the variable (or other guardrail?)
* implement control structures parsing, e.g. `if`, `while`, `break`, `continue`, `for`, ~~`?:`~~
* test the execution of whole scripts.
* make while/for/if etc into tokens, not identifiers
* implement ability to execute script by file
* implement loading script from file to execute (add line numbers as well)
* make "containable" something to pass to containers, not to the items themselves...
* implement function calling, for built-in functions, e.g.
`length()`, `str()`, `int()`, `float()`, `substr()`, `index()` etc
* parsing of expressions, using shunting yard, correct execution and saving!
* extract tests in their own files, go style
* split parser - lexer into directories
