# supported language

As this is an interpreter project, 
and the features of the supported interpreted language have become quite numerous,
I think keeping a reference of them is useful.

## general

In general, the interpreter supports a C-like language,
with similar syntax and constructs (C, C++, javascript, C#, java etc).
The language supports expressions, control flow statements,
functions, both named and anonymous. No module organization is setup yet.

While the language is not strongly typed (no explicit type declaration or checks), it has built in support for string, integer, float, boolean types, as well as lists, and dictionaries. No implicit type conversion is performed (e.g. it will _not_ do `5 + "1"`). Conditions always must yield a boolean result.

The code execution always returns a value (e.g. as in evaluating an expression),
akin to the exit status of executing a program in shell. It can be of any type,
NULL, int, string or even a list etc. The last "return" statement sets this value.

One can provide code of the language to execute in the command line (-e argument),
or in a script file (-f argument). The code is executed as encountered, no `main()` 
function is required.

## comments

Line comments start with double slash (`//`) and continue to the end of the line,
while block comments are enclosed by the slash-star combinations (`/* ... */`).

## literals

The following literal types are supported:

| Type | Format |
|----|----|
| Identifier | Any identifier starting with a letter [a-zA-Z] and containing letters, digits, and underscore. Identifiers are case sensitive. |
| String literal | Any string enclosed in single or double quotes |
| Number literal | Any number consisting purely of digits 0-9 and possibly the dot. A dot will yield a floating number, lack of dot yields an integer |
| Boolean literal | The keywords `true` and `false` |


## expressions

The usual expressions are supported out of the box. In fact, expressions 
can be residing in the body of a file, they don't have to be part 
of a function. Expressions end with a semicolon, or by the end of input, to allow
a parsing and execution of a simple expression (e.g. `"a + b"`).

The supported expressions are:

| Operator | Description |
|------|------|
| = | Assignment. Creates a local variable if name does not exist. |
| + | Addition (integers, floats, concatenation of strings) |
| - | Subtraction or negative numbers |
| * | Multiplication |
| / | Division, for integers and floats. Division by zero terminates the script |
| % | Modulo: remainder after integer division |
| ++, -- | Pre and post increment or decrement of a value, similar to C |
| <<, >> | Integer shift left or right |
| ( ... ) | Subexpression, or calling of a function |
| [ n ] | List item accessor, n is an integer |
| . m | Dictionary member accessor, m is an identifier |
| &&, \|\|, ! | Logical AND, OR, negation |
| &, \|, ^, ~ | Bitwise AND, OR, XOR, negation |
| ==, != | Equal and unequal value test. Operands must be of same type, e.g. int, str, list etc. No implicit type conversion applied. |
| >, <=, <, <= | Greater, greater-or-equal, less, less-or-equal value tests |
| [ operand, ... ] | Initialize a list |
| { key: operand, ... } | Initialize a dictionary |
| ? : | Short-hand 'if' expression |

## statements

Statements allow for control flow, branches, loops, function building.
Blocks of statements may be enclosed in brackets (`{...}`).

The following statements are supported:

| Statement | Description |
|------|------|
| if (cond) stmt | Conditional execution of statement or statement block. An optional `else` allows for alternative execution, if condition is not met. |
| while (cond) stmt | Repetitive execution of statement or statement block, for as long as the condition passes. |
| for (init; next; cond) stmt | Convenient loop writing, similar to other C-like languages.
| break | Break out of a loop. |
| continue | Repeat the loop. In the case of `for`, the `next` expression is evaluated |
| return | Can be with or without a value. Returns from the current execution scope with the optional value |
| function | Creates a file-level named function, or provides an anonymous function as an expression | 

## supported types

The supported, built-in types are the following:

| Type | Description |
|----|----|
| Bool | A boolean value, true or false |
| String | Any string enclosed in single or double quotes. |
| Integer | An integer value |
| Float | A floating value |
| List | A list of values. A list can contain different type lists, e.g. [ 1, false, "three" ] |
| Dict | A dictionary of values, keyed by string identifiers. Again the values can be of any type. Dictionary entries can be accessed by the dot operator, emulating object notation. Functions that are members of dictionaries, when executed, shall have the `this` variable pointing to the dictionary. |
| Callable | Something that can be called. It may take a list of arguments, even a named dictionary, and it can return any type, including lists, dicts, or callables |

## built in functions

The following built in functions are supported per type:

* lists
  * empty()
  * length()
  * add(item)
  * [n] -- retrieves the item
* dicts
  * empty()
  * length()
  * contains(key)
  * .m -- retrieves the member
* strings
  * empty()
  * length()
  * contains(substring)
  * indexof(substring)
  * substr(start, length)

To iterate over a list, use `for`. To iterate over a dict, use a ... ???

Apparently, we need to extend the above to time, random, environment variables, file manipulation etc.

