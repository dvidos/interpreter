# debugger

The idea was to have something similar to gdb or the Python debugger.

It is simple in conception, as all one has to do is check whether
a specific condition is met (e.g. a breakpoint has been reached),
and the run the interactive debugger command loop, 
passing in the current execution environment.

```
Inline debugger. Enter 'h' for help.
 38    -->   a = 1;
debug: h
  n -- next (run till next line number)
  s -- step (run till next statement or expression)
  r -- return (run till next return statement)
  c -- continue execution
  q -- quit execution

  l -- list code, l [+-]<line_no> [, <lines_to_show> ]
  a -- print function args and current values
  w -- where (print stack trace)

  b [ func_name | [file:]line_no ] -- toggle breakpoint to function or file:line
  p expresion -- evaluate and print expression
debug: 
```


## stepping

The stepping mechanism is implemented using checks 
before executing a statement or expression. 
There are the following options:

* step - sets a flag to enter debugger at the next check point.
* next - sets a flag to enter debugger when at a different file:line than the one the command was issued on
* return - sets a flag to enter debugger at the next `return` statement
* continue - clears flags and returns from debugger for execution
* about - debugger returns an 'Execution aborted' failure message

## stack trace

Having access to the execution context and the stack of the stack frames,
it is easy to print the stack.

```
Inline debugger. Enter 'h' for help.
  9    -->       if (i % 3 == 0 && i % 5 == 0)
debug: w
    3   fizzbuzz(), at scripts/script2.scr:8:9
    2   show_fizz_buzz(), at scripts/script2.scr:19:9
    1   run_demos(), at scripts/script2.scr:33:9
    0   scripts/script2.scr
```

## expression evaluation

Expression evaluation is yet another evaluation of the expression,
using the symbols that exist in the debugged environment:

```
Inline debugger. Enter 'h' for help.
 40    -->   c = 'some string';
debug: l
 38          a = 1;
 39          b = 2;
 40    -->   c = 'some string';
 41          
 42          breakpoint;
debug: p a+b
3
debug: 
```

## breakpoints

A list of breakpoints is maintained and manipilated by toggling 
a breakpoint at specific line numbers. In parallel to the above,
actual ST_BREAKPOINT statements are inkected and extracted in/from the 
Abstract Syntax Tree, so that debugger is engaged when they are encountered.
