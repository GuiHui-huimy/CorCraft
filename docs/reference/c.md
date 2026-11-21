# C Reference

This page is the one I open while writing, not while reading, so it is organised by the mistakes rather than by the standard. Widths, then the precedence rows people actually get wrong, then conversions, then the headers this book uses, then qualifiers, then the short list of undefined behaviour, then one page of rules I apply without thinking.

A lookup table, not a tutorial. Every row here has a chapter that explains it, and the chapter is what to read when the row does not settle the question.

## 1. Types and widths

| Type | x86-64 Linux | Windows MSVC | Minimum guaranteed |
| --- | --- | --- | --- |
| `char` | 1 | 1 | 1 |
| `short` | 2 | 2 | 2 |
| `int` | 4 | 4 | 2 |
| `long` | 8 | 4 | 4 |
| `long long` | 8 | 8 | 8 |
| `float` | 4 | 4 | 4 |
| `double` | 8 | 8 | 8 |
| `long double` | 16 (80-bit used) | 8 | 8 |
| pointer | 8 | 8 | ≥ 2 |

The `long` row is the one that breaks code: on Linux it is 64-bit, on Windows it is 32-bit, and a file format or a network protocol that assumes either will fail on the other. Use `int64_t` when the width matters.

```c
#include <stdint.h>
int8_t  int16_t  int32_t  int64_t      /* exact width, present or absent */
int_fast32_t  int_least16_t            /* at least this wide, fastest / smallest */
size_t  ptrdiff_t                      /* unsigned length, signed difference */
```

`size_t` is the unsigned type of `sizeof` and of array indices; `ptrdiff_t` is the signed type of pointer difference. Mixing them in a comparison produces `-Wsign-compare`, which is the warning that exists because the mixed comparison is usually a defect.

## 2. Operator precedence, the rows people get wrong

| Precedence | Operators | Associativity |
| --- | --- | --- |
| 1 | `()` `[]` `.` `->` | left |
| 2 | `!` `~` `++` `--` `+` `-` (unary) `*` `&` `sizeof` `(cast)` | right |
| 3 | `*` `/` `%` | left |
| 4 | `+` `-` | left |
| 5 | `<<` `>>` | left |
| 6 | `<` `<=` `>` `>=` | left |
| 7 | `==` `!=` | left |
| 8 | `&` | left |
| 9 | `^` | left |
| 10 | `\|` | left |
| 11 | `&&` | left |
| 12 | `\|\|` | left |
| 13 | `?:` | right |
| 14 | `=` `+=` `-=` … | right |
| 15 | `,` | left |

The three traps: `&` binds tighter than `==` (`x & 1 == 0` is `x & (1 == 0)`, always zero), `<<` binds tighter than `+` (`1 << 2 + 1` is `1 << 3`), and `*` in a declaration binds to the name (`int *a, b` declares one pointer).

## 3. Conversion rules

- Integer promotion: `char` and `short` promote to `int` in arithmetic.
- Usual arithmetic conversions: `int` + `unsigned int` → `unsigned int`, which is the defect where `-1 > 0u` is true.
- Integer to floating: exact up to 2²⁴ for `float` and 2⁵³ for `double`.
- Floating to integer: truncation toward zero, undefined if out of range.

`-Wconversion` reports every one of these that narrows. The book's labs are built with it.

## 4. Standard headers actually used

| Header | Used for | Chapter |
| --- | --- | --- |
| `stdio.h` | `printf`, `fopen`, `fread`, `getline` (POSIX) | files and streams |
| `stdlib.h` | `malloc`, `free`, `qsort`, `strtol`, `exit` | heap, sorting |
| `string.h` | `strlen`, `memcpy`, `strncpy`, `strcmp` | strings |
| `stdint.h` | fixed-width integers | bit operations |
| `stddef.h` | `size_t`, `ptrdiff_t`, `offsetof` | structs |
| `limits.h` | `INT_MAX`, `CHAR_BIT` | overflow |
| `errno.h` | `errno`, `strerror` | error handling |
| `assert.h` | `assert` (compiled out under `NDEBUG`) | testing |
| `time.h` | `clock`, `clock_gettime` | performance |
| `math.h` | `sqrt`, `atan2`, `fabs`, `isfinite` | geometry |

`getline` is POSIX, not ISO C: it needs `_POSIX_C_SOURCE` on strict builds and is absent on MSVC. The portability note is in the FASTA chapter.

## 5. Qualifiers

| Qualifier | Meaning | Trap |
| --- | --- | --- |
| `const` | not modified through this name | does not make the object immutable |
| `volatile` | may change outside the program's control | not a thread primitive |
| `static` (file scope) | internal linkage | not "persistent" |
| `static` (block scope) | static storage duration | initialised once, not per call |
| `restrict` | no aliasing through this pointer | violating it is UB |
| `register` | obsolete | ignored |

## 6. Undefined behaviour, the short list

Signed overflow, shift by ≥ width or negative, null or misaligned pointer access, out-of-bounds array access, `free` of a non-heap pointer or double free, reading an uninitialised object, modifying a string literal, `memcpy` with overlapping regions. Each has a row in [Undefined Behaviour](../c/undefined-behavior.md) with the detector that catches it.

## 7. The one-page rule set

1. Every allocation has exactly one owner and one `free`.
2. Every array index is checked by the function that computes it.
3. Every function returning a pointer documents how long the pointer is valid.
4. Every `sizeof` is applied to the object, not the type: `malloc(n * sizeof *p`.
5. Every integer that can be large is `size_t` or `int64_t`, not `int`.
6. Every comparison between signed and unsigned is cast explicitly.

These are the six rules behind every Field Note in the C chapters, written as rules rather than as stories.

## Continue

- [C++ Reference](cpp.md) is the same table for the other language.
- [Compiler Flags](compiler-flags.md) is how these warnings are turned on.
- [Objects in Memory](../c/objects-in-memory.md) is the chapter for the type table.
