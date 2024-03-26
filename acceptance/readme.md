# acceptance tests

This folder contains files describing acceptance tests that the interpreter takes.
Each file contains one or more acceptance tests.


Each acceptance test does the following:

* prepares possible input variables,
* grabs and executes the code
* validates outcome:
  * whether an exception is expected or not
  * the actual result returned from the code evaluation
  * the value of various input/output variables
  * the possible log contents

Each test has the following structure:

```
# title
// comment
code <code under test>
set a=1, b=2
expect result <expected result>
expect exception
expect log ...
ensure a=4, b=5
---
```

Individually:

* `//` define comments
* `#` optional title of the test
* `code` describes the code to run
* `expect result` describes the expected resulting value
* `expect log` describes the log contents to expect
* `set` describes variables to set before execution
* `ensure` describes variable values expected after execution
* `---` three or more dashes separate the tests

Code and log can be multiline, by using triple backticks and the end-of-text indicator, as below:

~~~
code ```
    for (i = 0; i < 10; i++)
        log(i);
```
~~~
