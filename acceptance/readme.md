# tests

Wanting to create a system for easy tests. Imagining a folder 
full of files, each describing one or more tests.

Each test has the following structure:

```
# comments
code: <code goes here (or can be multi line, using tripple quotes or indentation?)>
output: a=1, b=2
result: <expected result>
or expected-exception:
or expected-log: ...
expected-output: a=1, b=2
```

---

## title
```
code goes here
```
expectations:
* expect output: `output`
* expect log:
```
log contents
```
* expect exception



