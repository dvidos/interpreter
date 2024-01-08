# implementation in C

Maybe i could (for the fun of it) code something in C. 
An expression evaluator.

## purpose

* Read a tree of values
* Perform complex iterations, conditions, text manipulations
* Output text based on input plus manipulations.

## functionality

Functionality could include:

* A functionality to parse and evaluate expressions
  * e.g. "a > b" or "c != null"
  * it should support many operators with precedence
  * to be used in conditional expressions
  * values would be stored in a dictionary of variants
* A variant type
  * Initial value from string-based representation
  * ability to be converted in any type (number, boolean etc)
* A dictionaty type
  * Keys will be strings, values will be ~~variants~~ anything
  * Can be used to emulate objects
* An array type
  * Values will be ~~variants~~ anything
* Iterators over dictionaries and arrays
* Templating language / macros
* ...eventually, if we add function calls, this becomes an interpreter...?



