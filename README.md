# poise

The Poise programming language.

This is a rewrite of [grace](https://github.com/ryanjeffares/grace) because grace was a mess (the name is a synonym).

## Issues/Things on the Long Finger
* Parse argv properly, main function/file in general
* Testing
    * Test individual important functions
    * Fuzzing
* Think about object -> bool conversion
* Generally improve compiler errors and stop it from spitting out nonsense after one error, eg lambda capture errors
* Use hashes instead of strings in runtime
    * Function names, namespace lookups, etc
    * And maybe also in the compiler when we need to optimise that
* Package manager type thingy
    * Type checker for those optional type hints
    * Project manager/runner
    * Dependency stuff (not that we have the budget for that)
* Compiler warnings
* Optimisation
* ~~`getCodeAtLine()` for imported files~~
* Full UFCS + `Any` type annotation?
    * In general, I think I prefer the idea of having an `Any` type annotation as opposed to full UFCS.
    * However, adding an extension function to `Any` gets complicated.
    * Currently, each type has a list of its extension functions. Keeping this system, an extension function on `Any` would have to be added to all current and future types when compiled
    * We'd have to reverse this association, so each function knows which types it extends, but this is tedious to look up at runtime
    * So maybe the current system is ok? A `PoiseFunction` instance in a `Value` is basically a shared pointer, so it's not too crazy
* Simplify data structures in `NamespaceManager`
    * Reuse the string/ID map from the string interning, have a single data structure with all the info
* Namespace qualified calls are a little messy

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
    * ~~When we do constants, handle a constant value for construction Exceptions~~
    * Print only the first few call stack entries on error
    * ~~`throw` statement~~
* ~~If statements~~
* ~~While loops~~
* ~~Native functions~~
    * These are done, just need to actually implement everything else
    * ~~Disallow calling a native function outside std files~~
* ~~Imports + Namespaces~~
    * ~~Wildcards~~
    * ~~Multiple imports in `{}`~~
    * Back a folder
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
    * ~~Iterable collections~~
        * ~~Tidy up access modifiers~~
        * ~~Lists~~
        * ~~Ranges~~
            * ~~Can't change the internal state of the range so that you can iterate over it many times, or concurrently~~
        * ~~Tuple~~
        * ~~Dicts~~
        * ~~Sets~~
        * ~~Index operator~~
    * ~~Special constructors for the above~~
    * ~~Binary ops for the above~~
    * Could we do some kind of rust/c# linq/ranges style lazy evaluation...
    * ~~Option and Result types?~~
        * ~~No need, `try` assignments work like Results, everything is an option because anything can be `none` - implement `is_none()` and `is_some()` on `Any`~~
* ~~For loops~~
    * ~~Make sure everything's working fine for more collections we add~~
* ~~Zig style try assignment~~
    * ~~Could this be like a unary operator as opposed to only valid on assignment, so `foo(try bar())` is valid?~~
* Parameter packing and unpacking
    * ~~Expand unpacking into collections~~
        * ~~Remember this when we add Dicts and Sets...~~
    * ~~Unpack a collection~~
    * Multiple assignments on one line (`a, b = b, a`) or assigning an unpack (`a, b = ...collection`)
        * Partial unpacking - `var (a, b, ...rest) = ...collection` unpacks the first two items into `a` and `b` and the remaining into `rest`.
        * How to parse `var a, b, c = try ...expr`?
        * How badly do we want multiple assignments? Right now it works for variable declarations, but not for...
            * Reassigning variables
            * Assigning indexing etc
            * It would also imply the ability to return multiple values from a function
        * I'm just hesitant because of how annoying it is to compile LOL but maybe there's a better way to handle assignments in general in the compiler
    * ~~Friendship ended with `PoisePack`, `PoiseList` is my best friend now~~
    * ~~A "pack" will not be a unique type, and you will simply be able to unpack any collection~~
* Construct `Type` instance, `Type` ident
* ~~Union type annotation so that we can implement functions on multiple collections~~
    * Have type aliases, eg `type OrderedCollection = List|Range`
    * You should be able to export and import these
    * Maybe leave this until we do user defined classes, and we can come up with a nice generic way to handle it, since we'll have to work off of identifiers rather than builtin type keywords.
* ~~Break/continue statements~~
* Classes
    * Member variable access as well as extension function access
    * Need to generate `PoiseType` instances for these, and hook them into everything else...
* ~~GC for cycles~~
    * ~~Investigate horrible performance with Clang with cycles.poise (n=10000)~~
    * Ah, so using `unordered_set` instead of `vector` solved this
    * But, I bet performance is better for small amounts of objects - test this, and decide which should we optimise for? Is swapping between the two depending on the load viable?
* ~~Binary/Hex literals~~
* ~~Digit separators~~
* ~~Constants~~
    * ~~Constant expressions~~
    * ~~Type checked and folded into bytecode~~
* Crashes in compiler on EOF token
* Pattern matching
* Ifs as expressions
    * Do we want this, or some kind of ternary mechanism?
* ~~Identifier loading in runtime can be simplified now that strings are interned~~
* ~~Single expression lambdas~~
    * ~~But, perhaps we could do `|| => <expr>` and then also have single line functions like that too (`func foo() => <expr>`)~~
    * ~~Figure out how to compile this as a statement rather than an expression - need to return null in case of a print or loop statement etc.~~
* ~~Type hints~~
    * Expand to include user defined classes when we do these
        * Could we compile time check for the existence of the class? Is there a situation where you'd need to define a function that takes a class, before defining that class?
        * Yes, for cyclic dependant files.
    * ~~Allow >= 2 generic types for dictionaries, tuples etc when we do those~~
* CL arg parsing
* Standard Library
    * Precompile as bytecode files

## Iterable Inheritance Diagram
```
Object
 |
 |   _______________________ Iterable
 |__|_______|__________         |
 |  |   |   |   |______|____ Hashable
 |  |   |   |   |  |   | |
 List   Range   Dict   Set
```

