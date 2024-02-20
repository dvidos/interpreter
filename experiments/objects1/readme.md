# objects1

In this folder is an effort to make a strongly typed framework for objects in C.

Essentially, each "child" class duplicates the contents of the base class,
so that when we cast the pointer to either the base or the child class,
both work.

Also, the first item in a class is the pointer to class,
to allow the containers to work with it, without knowing the discrete type.

## experiments

Using this code:

```c
person *bob = new_person("bob");
act_on_person(bob);

student *maria = new_student("maria");
act_on_person((person *)maria);
act_on_student(maria);

act_on_unknown(bob);
act_on_unknown(maria);
```

We get this result:

```
I had a dream today...   <-- acting on person
person is 'Hi, I'm bob'

I took a test today...   <-- acting on person, actually on a student
person is 'Hi, my name is maria, my grade is 50'

I took a test today...   <-- acting on student
student is 'Hi, my name is maria, my grade is 90'

// acting on unknown / untyped instances
acting on instance of class 'person', description is 'Hi, I'm bob the builder', hash is 0x617cfa2f
acting on instance of class 'student', description is 'Hi, my name is maria, my grade is 90', hash is 0x583d7025
```

## requirements

In the header, we define:

* structure, with members (1st pointer to class, 2nd pointer to vtable)
* vtable structure with pointers to functions
* extern variable, pointer to the instance of the class info
* declaration of the public constructor function(s)

In the source file, we should define:

* all private (static) methods of the class
* all the private (static) class methods (e.g. describe, hash, clone)
* public initalized pointer to the instance of the class info
* the private (static) pointer to the instance of the vtable
* the body of the constructor(s)

Extra notes:

* the child classes must store the same information 
as the base class, in the same places. changing one parent
means we must change all children!
* the child struct cannot contain the base struct as is,
we need to copy its contents,
because the vtable is strongly typed and should be the child's type
* the child vtable cannot contain the base vtable as is,
we need to copy the functions, because the original functions
take the base type as arguments and the compiler issues a warning
* we can put the members of base class structures (istance and vtable)
in a pair of `#define`s, to allow us to easily embed them in the children classes
