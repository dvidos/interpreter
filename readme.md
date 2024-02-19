# interpreter

It started as a simple mental exercise, to see if we could parse 
and calculate an expression in the style of `1+2*3+4`, or `8-4-2`.

The initial idea was either to execute an Excel style formula,
or apply a calculation to big data, or something.

It has evolved into an interpreter with built in support for lists and dictionaries, running an expression in a string, or loading from script files. It offers an interactive debugger and an interactive shell.

```
$ ./ipret -h
A small C-like interpreter, an exercize to learn
Usage: ipret <options>
Options:
  -f <script-file>    Load and interpret a script file
  -e <expression>     Interpret and execute the expression
  -i                  Start interactive shell
  -d                  Enable debugger
  -b                  Show built in functions
  -v                  Be verbose
  -u                  Run self diagnostics (unit tests)
  -h                  Show this help message
  -q                  Suppress log() output to stderr
  -l <log-file>       Save log() output to file
```

## work description

Process for reading the script into an Abstract Syntax Tree:

* A **lexer** is converting characters into tokens. We create a trie for fast lookup.
* A **statement parser** is parsing statements (e.g. if, while, for, etc)
* An **expression parser** parses expressions (e.g. a=1, func() etc)

Process for executing the Absract Syntax Tree:

* A **statement executor** is executing the statements (blocks, loops, break, continue, return)
* An **expression executor** is executing the expressions (assignments, math, comparisons, function calls)
* In order to load and save values of variables we use a **symbol table**. One is created for every function we enter. If the function was anonymous member of a dictionary, the `this` symbol points to that dictionary. This emulates objects, similar to javascript.
* There are three types of functions supported:
  * **built in** functions: strpos(), strlen() etc.
  * **user defined** functions, as a statement, at a script body
  * **anonymous functions**, as an expression operand

* An integrated **debugger** enabled by the `-d` flag, and the `breakpoint;` keyword in the code. It allows to examine code, variables, set breakpoints and change values.
* An integrated **interactive shell** that runs the interpreter 
in a loop, for each expression by the user. 
Values are preserved between executions.

