# todo

* implement print() or something for std out (maybe streams is better?).
* make iterator a two level object, internal is move/validate only, externa supports:
`last`, `curr`, `peek` etc, to aleviate code repetition. Then, since we have the `last` method, fix:
  * the `if(a;b;c;d) {...} case
  * function arguments parsing, so that parser does not need to store last token.
* implement parsing of functions

* make parsing of dictionaries `a = {key: value, ...}` and lists `b = (1, 2, ...)`
* make calling of user defined functions.
* find a way to implement objects using dictionaries and the `this` keyword.
* make better iterators, minimal ask from conntainers.
* make streams for execution, stdin/out etc?
* could implement a `foreach` keyword, to iterate over 

* instead of multiple "switch" statements to handle polymorphic behavior,
convert to function pointers, and implement discrete functions for each case.
This should speed up runtime a ton.


## later

Ability to parse EBNF syntax and then parse things on the fly?



# done!

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
