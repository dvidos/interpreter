# todo

* merge execution_outcome with variants. 
* exceptions need 

* implement method calling on data types.
  * for lists: empty, length, add, filter, map, reduce, foreach.
  * for dicts: empty, length, set, get, keys, values (make dict dup the key)
  * for numbers: abs(), factorial()
  * for strings: empty, length, substr(), find(), left(), right(), startswith(), endswith() etc
  * kill most of the built in functions
  * implement tests for all the above using the log or something

* parse `class` with functions and attributes, private is the default mode, 
`public` opens up visibility to others, create `variant_type`s on the fly.

* convert to strongly typed language????

* implement read only execution mode, where the root symbols are never changed.

* cpython, inner code, gets the intepreter state and the current thread's state
(that includes the current stack frame) by calling a function, which refers 
to a static variable inside the C file. This way, call mechanism 
does not need to contain contexts etc.

* see that maybe the class_info can give us a dictionary
of primitive actions, e.g. "factorial" for the integer variants.
In that sense we could write: `5.power(2);` or `4.fact();`,
same as we'd do `[4,1,3,2].sort();`, or `'hello'.print();`.

* write tests to prove scope visibility and symbol resolution.

* find a way to make `foreach()` iterators, and allow scripted objects
to expose iterators behavior. For example to expose reset/curr/move.

* continue with the adventure game, it unveils good improvements.

* write a meaningful / useful script? what should this be used for?
  * ideas: tic-tac-toe, othello, score-4, rock-paper-scissors, 
  * text based adventure? make a random map with random items (keys, chests, doors) and allow user to play and print the map.
  * sort, grep, ?
* reorg folders into src, scripts, docs etc?
* where do we go with this? graphics? pipelines?

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

* try to see if we can make an edit-compile-run cycle in VSCode by using the intepreter!!


## later

* Maybe have an interactive way to navigate the AST code and structures...
* Maybe start implementing memory management for the exeuted code, therefore
keeping track of variables and memory, ala Rust.
* Ability to parse EBNF syntax and then parse things on the fly?
* Make a debugger, with ability to step, and investigate variables!!!!!



# done!

* continue to old-style variants (union-based) convert types from `variant_original` to the new variant types (type-based)
* implement exceptions in the script language
* maybe see if we can make an interactive mode (-1),
similar to python, where the user writes expressions
and there is a frame that holds variables and values.
* make testing uniform. fix the "testing" package to serve.
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
