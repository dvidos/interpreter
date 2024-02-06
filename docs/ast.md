# AST

The abstract syntax tree that this program loads and executes consists of the following:

## statements

When a piece of code is interpreted, it ends up as a list of statements. A statement can be the following:

* `ST_EXPRESSION` - contains one expression
* `ST_IF`
  * contains one expression for the condition
  * a list of statements for the body, for when condition is true
  * a flag of whether an "else" case is defined, when condition is false
  * a possible list of statements for the "else" case
* `ST_WHILE`
  * contains one expression for the condition
  * a list of statements for the body
* `ST_FOR_LOOP`
  * contains three expressions for: 
    * `init` - executed once before the loop
    * `condition` - executed each time before the loop
    * `next` - executed each time after the loop, and after a `continue` statement
  * a list of statements for the body
* `ST_CONTINUE` - has no attributes
* `ST_BREAK` - has no attributes
* `ST_RETURN` - contains a possible expression for the returned value
* `ST_FUNCTION`
  * contains a name (since it is defined in a statement)
  * a list of argument names
  * a list of statements to execute

## expressions

If a token is not parseable as a statement, it will be attempted to be parsed as an expression. An expression can be the following:

* `ET_IDENTIFIER` - e.g. a symbol name
* `ET_NUMERIC_LITERAL` - e.g. 123 or 3.14
* `ET_STRING_LITERAL` - in single or double quotes
* `ET_BOOLEAN_LITERAL` - the keywords `true` and `false`
* `ET_UNARY_OP` - a unary operator (list to come), with one operand. Operators can be:
   * `OP_POST_INC`          (a++)
   * `OP_POST_DEC`          (a--)
   * `OP_PRE_INC`           (++a)
   * `OP_PRE_DEC`           (--a)
   * `OP_POSITIVE_NUM`      (+a)
   * `OP_NEGATIVE_NUM`      (-a)
   * `OP_LOGICAL_NOT`       (!a)
   * `OP_BITWISE_NOT`       (~a)
* `ET_BINARY_OP` - a binary operator, with two operands
  * The specially handled binary operators are:
    * `OP_FUNC_CALL`          (f()) - right operand is a list of argument expressions
    * `OP_ARRAY_SUBSCRIPT`    (a[b]) - right operand is an expression yielding an integer
    * `OP_MEMBER`             (a.b) - right operand is an identifier
    * `OP_SHORT_IF`           (a ? b : c) - right operand is a list of two expressions [b, c]
    * `OP_ASSIGNMENT`         (a = b) - creates the variable if it's not there [^1]
  * More straightforward binary operators are:
    * `OP_MULTIPLY`           (a * b)
    * `OP_DIVIDE`             (a / b)
    * `OP_MODULO`             (a % b)
    * `OP_ADD`                (a + b)
    * `OP_SUBTRACT`           (a - b)
    * `OP_LSHIFT`             (a << b)
    * `OP_RSHIFT`             (a >> b)
    * `OP_BITWISE_AND`        (a & b)
    * `OP_BITWISE_XOR`        (a ^ b)
    * `OP_BITWISE_OR`         (a | b)
    * `OP_LOGICAL_AND`        (a && b)
    * `OP_LOGICAL_OR`         (a || b)
  * The following are dual: operation and assignment:
    * `OP_ADD_ASSIGN`         (a += b)
    * `OP_SUB_ASSIGN`         (a -= b)
    * `OP_MUL_ASSIGN`         (a *= b)
    * `OP_DIV_ASSIGN`         (a /= b)
    * `OP_MOD_ASSIGN`         (a %= b)
    * `OP_RSH_ASSIGN`         (a >>= b)
    * `OP_LSH_ASSIGN`         (a <<= b)
    * `OP_AND_ASSIGN`         (a &= b)
    * `OP_OR_ASSIGN`          (a |= b)
    * `OP_XOR_ASSIGN`         (a ^= b)
  * The following operations are comparisons, yielding a boolean result
    * `OP_LESS_THAN`          (a < b)
    * `OP_LESS_EQUAL`         (a <= b)
    * `OP_GREATER_THAN`       (a > b)
    * `OP_GREATER_EQUAL`      (a >= b)
    * `OP_EQUAL`              (a == b)
    * `OP_NOT_EQUAL`          (a != b)
* `ET_LIST_DATA` - a list data (e.g. a list initializer), with expressions for values.
* `ET_DICT_DATA` - a dictionary data (e.g. a dict initializer), with strings for keys and expressions for values
* `ET_FUNC_DECL` - an anonymous function declaration. it contains:
  * a list of argument names
  * a list of statements to execute

[^1]: Here lies the destinction if variables are to be declared first.
It's a delicate balance between flexible but unsafe code and more rigid but safer code.

