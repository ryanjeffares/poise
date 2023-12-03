# poise

The Poise programming language.

This is a rewrite of [grace](https://github.com/ryanjeffares/grace) because grace was a mess (the name is a synonym).

## Issues
* Parse argv properly, main function in general
* Error Handling
    * Option/Result types as built in language types
    * Eh, still need exceptions that can be thrown internally for binary operations for example
    * Would it be messy to mix these two approaches in the language core/stdlib?
* Testing
    * Test individual important functions
    * Fuzzing
* Think about object -> bool conversion
* Think about imports
    * Could do simple file opening during compilation, but how do we resolve what has been imported during runtime?
        * I GUESS we could have some sort of map, file -> imported namespaces if we can keep track of what file the currently executing function is in during the runtime
        * Yeah I think I like this
    * Or a Python style thing where a `namespace` is a type
* Generally improve compiler errors and stop it from spitting out nonsense after one error, eg lambda capture errors

## Roadmap
* ~~Pop unused expression/return results~~
* ~~Create values by calling type as a constructor, eg `Int()`, `Float(1.0)`~~
* ~~Implement types as first class objects - see below~~
    * Done except for user defined classes when we get to that...
* ~~Assertion failure in formatting compiler error output for missing semicolons~~
* ~~Expressions with primitives~~
    * ~~Short-circuiting~~
* ~~Variables~~
* Functions 
    * ~~Declaration~~
    * ~~Calling~~
    * ~~Returning~~
    * ~~Lambda~~
* Assignment statements
* Error handling
* If statements
* While loops
* Native functions
* Imports + Namespaces
* Builtin objects
    * Iterable collections
        * Lists
        * Ranges?
        * Dicts
            * Pair
        * Sets
        * Ranges
    * Option and Result types
* For loops
* Classes
* GC
* Type hints
* CL arg parsing

### Types as First Class Objects
* So, a variable can hold an instance of a `PoiseType` which describes some type
* Need name, primitive/object
* Constructing any object can be done by calling an instance of its type like a constructor:

```
final t_list = typeof([]);
final list_instance = t_list();

final t_int = Int;
int_value = t_int(5);
```

* To allow the above, naming a type (`Int`, `Float`, `MyClass` (if MyClass is defined)) without a call (`()`) will load an instance of `PoiseType` made for each of these
* For user defined classes, these will hold a `Value` which is the constructor function
* For builtins, there will just be some bytecode
* Cannot get type of a type, __for now__:

```
final t = typeof(Int);  // error - cannot take type of type
```

* This is **ALL DONE** now, just need to consider user defined classes and collections when we add them (and __maybe__ constructing Types themselves...)