# poise

The Poise programming language.

This is a rewrite of [grace](https://github.com/ryanjeffares/grace) because grace was a mess (the name is a synonym).

## Issues
* Think about how to treat functions and how to handle main function/top level code
* Parse argv properly, main function in general
* Error Handling
    * Option/Result types as built in language types
    * Eh, still need exceptions that can be thrown internally for binary operations for example
    * Would it be messy to mix these two approaches in the language core/stdlib?
* Testing
    * Test individual important functions
    * Fuzzing
* Think about object -> bool conversion
* Think about functions as first class objects

## Roadmap
* Expressions with primitives 🗸
    * Short-circuiting 🗸
* Variables 🗸
* Functions 
* Native functions
* Builtin objects
    * Iterable collections
        * Lists
        * Dicts
            * Pair
        * Sets
        * Ranges
    * Option and Result types
* GC