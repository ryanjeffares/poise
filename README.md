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
* Imports
    * The code is really messy
* Generally improve compiler errors and stop it from spitting out nonsense after one error, eg lambda capture errors
* Use hashes instead of strings in runtime
    * Function names, namespace lookups, etc
    * And maybe also in the compiler when we need to optimise that

## Roadmap
* ~~Pop unused expression/return results~~
* ~~Create values by calling type as a constructor, eg `Int()`, `Float(1.0)`~~
* ~~Implement types as first class objects - see below~~
    * Done except for user defined classes when we get to that...
* ~~Assertion failure in formatting compiler error output for missing semicolons~~
* ~~Expressions with primitives~~
    * ~~Short-circuiting~~
* ~~Variables~~
* ~~Functions~~
    * ~~Declaration~~
    * ~~Calling~~
    * ~~Returning~~
    * ~~Lambda~~
* ~~Assignment statements~~
* ~~Error handling~~
    * When we do constants, handle a constant value for construction Exceptions
    * Print only the first few call stack entries on error
* ~~If statements~~
* ~~While loops~~
* ~~Native functions~~
    * These are done, just need to actually implement everything else
    * Disallow calling a native function outside std files
* Imports + Namespaces 
    * A 'namespace' corresponds to a file and its relative path to the main file being run 
    * Aliases for QOL
* Builtin objects
    * Iterable collections
        * Lists
        * Ranges?
        * Dicts
            * Pair
        * Sets
        * Ranges
    * Option and Result types?
* For loops
* Break statements
* Classes
* Dot function calls/member access
* GC
* Constants
* Type hints
* CL arg parsing
* Standard Library
    * Precompile as bytecode files

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
* Cannot get the type of a type, _for now_:

```
final t = typeof(Int);  // error - cannot take type of type
```

* This is **ALL DONE** now, just need to consider user defined classes and collections when we add them (and _maybe_ constructing Types themselves...)