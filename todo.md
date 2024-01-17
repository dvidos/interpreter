# todo

* implement control structures parsing, e.g. `if`, `while`, `break`, `continue`, `for`, ~~`?:`~~
* make iterator a two level object, internal is move/validate only, externa supports:
`last`, `curr`, `peek` etc, to aleviate code repetition. Then, since we have the `last` method, fix:
  * the `if(a;b;c;d) {...} case
  * function arguments parsing, so that parser does not need to store last token.
* fix first assignment to create the variable (or other guardrail?)
* implement parsing of functions
* implement ability to execute script by file


## later

Ability to parse EBNF syntax and then parse things on the fly?



# done!

* implement loading script from file to execute (add line numbers as well)
* make "containable" something to pass to containers, not to the items themselves...
* implement function calling, for built-in functions, e.g.
`length()`, `str()`, `int()`, `float()`, `substr()`, `index()` etc
* parsing of expressions, using shunting yard, correct execution and saving!
* extract tests in their own files, go style
* split parser - lexer into directories
