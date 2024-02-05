# todo

* make testing uniform. fix the "testing" package to serve.

* separate the built in functions into separate files, based on the object they attach to.
* write tests to prove scope visibility and symbol resolution.

* find a way to make `foreach()` iterators, and allow scripted objects
to expose iterators behavior. For example to expose reset/curr/move.

* create built-in methods for the built-in types. See OP_MEMBER execution.
  * list: add, length, indexof, to_string
  * dict: add, set, contains, indexof, to_string
  * str: length, substr, indexof, split, etc

* continue with the adventure game, it unveils good improvements.

* write a meaningful / useful script? what should this be used for?
  * ideas: tic-tac-toe, othello, score-4, rock-paper-scissors, 
  * text based adventure? make a random map with random items (keys, chests, doors) and allow user to play and print the map.
  * sort, grep, ?
* reorg folders into src, scripts, docs etc?
* where do we go with this? graphics? pipelines?

* [maybe] consider removing getter functions and allow direct reading of struct members.
for speed. and simplicity.
* make better iterators, minimal ask from conntainers.
* make iterator a two level object, internal is move/validate only, externa supports:
`last`, `curr`, `peek` etc, to aleviate code repetition. Then, since we have the `last` method, fix the `if(a;b;c;d) {...} case
* find a way to implement objects using dictionaries and the `this` keyword.
* make streams for execution, stdin/out etc? streams should also work per 
object, e.g. a json object, not only per text line or bytes.
* could implement a `foreach` keyword, to iterate over dicts and lists
* instead of multiple "switch" statements to handle polymorphic behavior,
convert to function pointers, and implement discrete functions for each case.
This should speed up runtime a ton.

* experiment with idea of declaration & implementation of strongly typed containers through macros, akin to failables...


## later

* Maybe have an interactive way to navigate the AST code and structures...
* Maybe start implementing memory management for the exeuted code, therefore
keeping track of variables and memory, ala Rust.
* Ability to parse EBNF syntax and then parse things on the fly?
* Make a debugger, with ability to step, and investigate variables!!!!!



# done!

* create tests to prove the built in functions
* fix the context of a function call, we need the current and the global 
symbol table, not all the functions in between. We may also need the module table. Fix an object for the context (e.g. stack_frame) and stop using the 
stack of symbol tables.
* extract the various "types" (token type, statement type, expression type, 
value type) on separate files, to allow better management.
* MUST keep track in expressions or operators of the source file and line.
* improve the failable() with nesting
* make calling user functions and user methods possible.
* symbol tables should contain info about variable type or function type, flags etc.
* make function an expression, in order for it to be used as a value and passed around, 
returned from functions etc.
* implement parsing of functions
* make calling of user defined functions.
* fix / simplify both parsers (expression,statement) to have: accept(), peek(), accepted() etc.
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
