# poise

The Poise programming language.

This is a rewrite of [grace](https://github.com/ryanjeffares/grace) because grace was a mess (the name is a synonym).

## Issues/Things on the Long Finger
* Parse argv properly, main function/file in general
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
    * Give nicer compiler errors
* Generally improve compiler errors and stop it from spitting out nonsense after one error, eg lambda capture errors
* Use hashes instead of strings in runtime
    * Function names, namespace lookups, etc
    * And maybe also in the compiler when we need to optimise that
* Type checker for those optional type hints
* Compiler warnings
* Optimisation
* ~~`getCodeAtLine()` for imported files~~
* Think about whether you should be able to unpack anywhere
    * How would we know what to pop off the stack, if only one of the values is used?
    * Unpacking would need a different Vm implementation
* Vectors instead of maps!

## Feature Roadmap
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
    * ~~`throw` statement~~
* ~~If statements~~
* ~~While loops~~
* ~~Native functions~~
    * These are done, just need to actually implement everything else
    * ~~Disallow calling a native function outside std files~~
* ~~Imports + Namespaces~~
* ~~Namespace aliases~~
* ~~Export functions~~
* ~~Dot functions - UFCS!~~
    * ~~For imported functions...~~
        * ~~Put the namespace stuff into its own class~~
        * ~~Functions need to know what namespace they're in~~
        * ~~Types know what extension functions they have~~
        * ~~Check if that function's namespace has been imported to the namespace of the current function in the Vm~~
    * ~~Two functions with the same name in a different namespace will override each-other~~
* Builtin objects
    * ~~Use types::Type instead of Value::TypeInternal for type()~~ 
    * Iterable collections
        * Tidy up access modifiers
        * ~~Lists~~
        * ~~Ranges~~
        * Tuple
        * Dicts
        * Sets
        * Index operator
    * Special constructors for the above
    * Binary ops for the above
    * Could we do some kind of rust/c# linq/ranges style functor...
    * Option and Result types?
        * No need, `try` assignments work like Results, everything is an option because anything can be `none` - implement `is_none()` and `is_some()` on `Any`
* For loops
    * Make sure everything's working fine for more collections we add
* ~~Zig style try assignment~~
* Parameter packing and unpacking
    * ~~Expand unpacking into collections~~
        * Remember this when we add Dicts and Sets...
    * Unpack a collection
    * Return multiple values, this is a pack
    * Multiple assignments on one line (`a, b, = b, a`) or assigning an unpack (`a, b = ...pack`)
* Construct `Type` instance, `Type` ident
* `Any` type keyword for implementing dot functions on any variable
* Union type annotation so that we can implement functions on multiple collections
    * If we're going to do this, maybe reverse the type -> function association so that we're not duplicating functions
    * Also, when we revisit extension functions, do full UFCS
* Break statements
* Classes
    * Member variable access as well as extension function access
    * Need to generate `PoiseType` instances for these, and hook them into everything else...
* GC
* Binary/Hex literals
* Digit separators
* Constants
    * Constant expressions
* Pattern matching
* Ifs as expressions
* Single expression lambdas
* Type hints
* CL arg parsing
* Standard Library
    * Precompile as bytecode files

## Iterable Inheritance Diagram
```
PoiseObject
    |
    |   _______________________________________ PoiseIterable
    |__|___________|      |           |
    |  |       |   |      |   ________|________ PoiseHashable
    |  |       |   |      |  |        |   |
PoiseList   PoiseRange  PoiseDict   PoiseSet
```
