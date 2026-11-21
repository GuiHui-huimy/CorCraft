# Error Cookbook

Errors are indexed by the moment they appear, because that is how you meet them. Compile time in C, link time in C, compile time in C++, link time in C++, sanitizer reports at runtime, then wrong answers, which are the only ones with no message at all. Each entry is a real message with the cause underneath it.

Message first, cause second, chapter third. Every entry is a message this book's code has actually produced.

## 1. C: compile time

| Message | Cause | Read |
| --- | --- | --- |
| `warning: implicit declaration of function 'foo'` | missing `#include`, or the function is declared later | [translation units](../c/translation-units.md) |
| `error: 'foo' undeclared` | missing declaration, or a typo in the name | [translation units](../c/translation-units.md) |
| `warning: incompatible implicit declaration of built-in` | missing header for a standard function — the compiler guessed the signature | [strings](../c/strings.md) |
| `error: expected ';' before ...` | the error is usually one line earlier | — |
| `warning: unused variable 'x'` | a leftover; `-Werror` turns it into a stop | [first program](../start-here/first-program.md) |
| `warning: conversion to 'unsigned int' from 'int' may change the sign` | `-Wconversion`; a `size_t` compared with an `int` | [types](c.md) |
| `warning: comparison of integer expressions of different signedness` | same as above at a comparison | [undefined behaviour](../c/undefined-behavior.md) |
| `warning: format '%d' expects 'int' but argument is 'size_t'` | wrong format specifier; `%zu` for `size_t` | [files and streams](../c/files-and-streams.md) |
| `warning: control reaches end of non-void function` | a missing `return`; the behaviour is undefined | [error handling](../c/error-handling.md) |
| `error: array size missing in 'a'` | `int a[]` with no initialiser | [arrays and layout](../c/arrays-and-layout.md) |
| `error: invalid use of incomplete type` | a pointer to a struct whose definition is not visible | [translation units](../c/translation-units.md) |

## 2. C: link time

| Message | Cause | Read |
| --- | --- | --- |
| `undefined reference to 'foo'` | declared but never defined, or the object file is not on the command line | [compile pipeline](../engineering/compile-pipeline.md) |
| `undefined reference to 'main'` | no `main`, or it is `static` | [first program](../start-here/first-program.md) |
| `multiple definition of 'foo'` | a function defined in a header included by two units | [CMake](../engineering/cmake.md) |
| `undefined reference to 'sqrt'` | missing `-lm`, or `-lm` placed before the object file | [compiler flags](compiler-flags.md) |
| `undefined reference to 'printf'` with `-nostdlib` | the C library was removed deliberately | [compile pipeline](../engineering/compile-pipeline.md) |

## 3. C++: compile time

| Message | Cause | Read |
| --- | --- | --- |
| `no matching function for call to 'foo(int, double)'` | conflicting template deduction, or an ambiguous overload | [templates](../cpp/templates.md) |
| `'foo' was not declared in this scope` in a template | two-phase lookup: the name must be visible at definition | [templates](../cpp/templates.md) |
| `invalid use of incomplete type 'class Foo'` | forward declaration used where the definition is needed | [classes](../cpp/classes.md) |
| `cannot convert 'std::string' to 'const char*'` | `.c_str()`, and the pointer must not outlive the next modification | [strings](../c/strings.md) |
| `use of deleted function 'Foo(const Foo&)'` | the class is move-only; the call needs `std::move` | [RAII](../cpp/raii.md) |
| `no viable conversion from 'Foo*' to 'std::unique_ptr<Foo>'` | raw pointer to owning pointer needs an explicit constructor | [RAII](../cpp/raii.md) |
| `override did not override any member functions` | the name or signature differs in case or constness | [inheritance](../cpp/inheritance.md) |
| `delete of an abstract class` / `has virtual functions but non-virtual destructor` | deleting through a base without a virtual dtor | [polymorphism](../cpp/polymorphism.md) |
| `no match for 'operator<<'` | the stream operator is not declared for this type | [operators](../cpp/operators.md) |
| `call of overloaded 'foo(int)' is ambiguous` | two conversions reach the same rank | [cpp reference](cpp.md) |
| `the value of 'x' is not usable as a constant expression` | a `constexpr` context received a runtime value | [modern](../cpp/modern.md) |

## 4. C++: link time

| Message | Cause | Read |
| --- | --- | --- |
| `undefined reference to 'Foo::bar()'` | a member declared but not defined, or the definition is in a unit not compiled | [CMake](../engineering/cmake.md) |
| `undefined reference to 'vtable for Foo'` | the first virtual function is not defined | [polymorphism](../cpp/polymorphism.md) |
| `undefined reference to 'typeinfo for Foo'` | RTTI disabled on one side (`-fno-rtti`) | [polymorphism](../cpp/polymorphism.md) |
| `multiple definition of 'Foo::Foo()'` | an inline function defined in a header without `inline` | [CMake](../engineering/cmake.md) |

## 5. Runtime: sanitizer reports

| Report | Cause | Read |
| --- | --- | --- |
| `heap-buffer-overflow ... READ of size 4` | index or pointer one past the end | [heap](../c/heap.md) |
| `stack-buffer-overflow` | same, on an automatic array | [arrays and layout](../c/arrays-and-layout.md) |
| `heap-use-after-free` | a stale pointer or iterator | [heap](../c/heap.md) |
| `stack-use-after-return` | a pointer to a local returned (needs `-fsanitize-address-use-after-scope`) | [stack frames](../c/stack-frames.md) |
| `attempting free on address which was not malloc-ed` | `free` of a stack address or an interior pointer | [heap](../c/heap.md) |
| `alloc-dealloc-mismatch (operator new [] vs operator delete)` | `new[]` with `delete` | [RAII](../cpp/raii.md) |
| `detected memory leaks ... definitely lost` | an allocation with no owner | [heap](../c/heap.md) |
| `runtime error: signed integer overflow` (UBSan) | arithmetic outside the type's range | [undefined behaviour](../c/undefined-behavior.md) |
| `runtime error: shift exponent 32 is too large` | shift by ≥ the type width | [bit-level programming](../c/bit-level-programming.md) |
| `runtime error: load of null pointer` | dereference of `NULL` | [pointers](../c/pointers.md) |
| `SEGV on unknown address 0x000000000000` | null dereference without a sanitizer | [debugging](../engineering/debugging.md) |

## 6. Runtime: wrong answers

| Symptom | Cause | Read |
| --- | --- | --- |
| output contains `nan` | `acos` argument outside `[-1, 1]`, division by zero, or an uninitialised value | [geometry](../bio/geometry.md) |
| `-nan` | same, with the sign bit set — still a NaN, still the same causes | [numerics](../bio/numerics.md) |
| a benchmark prints `0.000 s` | the timer's resolution, or the work was optimised away | [performance](../engineering/performance.md) |
| a parallel speedup is below 1 | CPU time measured instead of wall time, or too little work | [parallel scoring](../bio/parallel-scoring.md) |
| a GC fraction is negative or above 1 | dividing by a count that includes non-residues | [sequence encoding](../bio/sequence-encoding.md) |
| a PDB parse merges chains | column off-by-one on the chain field | [PDB format](../bio/pdb-format.md) |
| an alignment is shifted by one | 0-based array with a 1-based loop | [alignment](../bio/alignment.md) |

## Continue

- [Debugging](../engineering/debugging.md) is what to do when the message is not in this table.
- [Compiler Flags](compiler-flags.md) is how to turn the warnings on.
- [Testing and Sanitizers](../engineering/testing.md) is how to reproduce a report.
