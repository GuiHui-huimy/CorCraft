# Translation Units

The compiler sees one unit and the linker sees symbols, and every error in this chapter lives in the gap between those two views. I start with the unit and the two stages that read it, then declaration versus definition, then `static` at file scope, then the two link errors people actually get. `inline` and the one-definition rule come after, because they are the modern version of the same problem, and the macro section is there because a macro is the only construct that can capture an argument twice.

A C program is not compiled as a whole. It is compiled as a set of translation units, each one a `.c` file plus everything it includes, and then the linker joins the results. Almost every confusing error in this chapter is a disagreement between what one unit believes and what another unit provides.

## 1. The unit, and the two stages that read it

```
foo.c  →  preprocess  →  foo.i  →  compile  →  foo.o  →  link  →  a.out
```

The preprocessor is textual: it copies headers, expands macros, and evaluates `#if`. The compiler sees one unit and knows nothing about the others. The linker sees symbols and knows nothing about types. So a function declared with one signature and defined with another compiles cleanly and fails at run time, and a variable defined twice fails at link time with a message that names the second definition, not the first.

## 2. Declaration and definition

```c
/* point.h */
#ifndef POINT_H
#define POINT_H

struct point { double x, y; };

double distance(const struct point *a, const struct point *b);  /* declaration */
extern int point_count;                                          /* declaration */

#endif
```

A declaration says a name exists and what type it has. A definition creates the object or the function body. The header holds declarations; exactly one `.c` file holds each definition.

The include guard is not optional. Without it, a header included twice in one unit — once directly, once through another header — defines `struct point` twice, which is a redefinition error. `#pragma once` does the same job in every mainstream compiler, is not standard C, and is used in this book's own headers because the duplicate-include case is entirely mechanical.

## 3. `static` at file scope means internal linkage

`static` on a function or a file-scope variable means "this name is not exported". The linker never sees it, so two translation units may each have a `static` helper with the same name and no conflict.

```c
/* LAB 13-01  translation_units/01_static_scope.c */
static int counter;                 /* one per translation unit */

static int bump(void) { return ++counter; }

int public_api(void) { return bump(); }
```

The variable is not shared with any other unit even if another unit declares `extern int counter`. That is the point: `extern` finds nothing, and the link fails rather than silently aliasing two unrelated objects.

The opposite case is the one that catches people. A header containing a definition — `int counter;` rather than `extern int counter;` — is included by three units, producing three definitions and a link error:

```
multiple definition of `counter'; first defined here
```

The repair is to keep the definition in one `.c` file and the `extern` declaration in the header. The C++ section solves the same problem with `inline` variables, which is a different mechanism for the same need.

## 4. Reading the two error messages

The two link errors mean different things and lead to different fixes.

```
undefined reference to `distance'
```

The linker found a call and no definition. Either the `.c` file holding the body is not in the build, or its name differs by case, or it is inside an `extern "C"` block in C++ that was omitted.

```
multiple definition of `counter'
```

The linker found two definitions. Either a header holds a definition, or a `.c` file was added to the build twice, or a `static` definition was accidentally made non-static.

Both are link-time, so both appear after every unit compiled successfully — which is why "it compiled" is not "it built".

## 5. `inline` and the one-definition rule

C99's `inline` is subtler than its C++ counterpart. A function declared `inline` in a header and used by three units may be emitted in each of them; the standard resolves this by requiring one unit to provide an external definition. Getting it wrong produces either a missing symbol or a duplicate one, depending on the compiler's mood.

```c
/* header */
static inline int clamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
```

`static inline` in a header is the practical form: each unit gets its own copy, the compiler inlines it, and no symbol is exported. It costs a few bytes of code duplication and removes the entire linkage question. This book uses it for the small helpers in `code/include`.

## 6. Macros that capture arguments

The preprocessor substitutes text, which means an argument is evaluated every time it appears:

```c
#define SQUARE(x) ((x) * (x))
int v = SQUARE(++i);      /* i incremented twice */
```

The double parentheses protect against precedence, not against double evaluation. The fix is a function, or an `inline` function, or a macro that evaluates its argument exactly once by way of a temporary in a statement expression — a GNU extension this book does not use.

The other macro trap is the missing parentheses around the whole body:

```c
#define DOUBLE(x) (x) * 2
int v = DOUBLE(1 + 2) * 3;   /* expands to (1 + 2) * 2 * 3 */
```

The body must be fully parenthesised and every parameter must be parenthesised. A macro that satisfies both conditions is still a candidate for replacement by `static inline`.

## 7. Field note: the header that defined a table

A project declared its lookup table in a header:

```c
/* lookup.h — the original */
const char *names[] = { "alpha", "beta", "gamma" };
```

Two units included it, both got a definition, and the link failed with `multiple definition of 'names'`. The author's first response was to add `static`, which silenced the linker and gave each unit its own copy — 3 copies of the table at run time, and a `sizeof` in one unit that disagreed with the array another unit had modified.

The correct repair was `extern const char *const names[];` in the header and one definition in a `.c` file. The lesson is that `static` is not a way to silence a link error; it is a statement that the name is private, and a table shared across units is not private.

## Continue

- [Function Pointers](function-pointers.md) explains why a signature mismatch links and fails at run time.
- [Compile Pipeline](../engineering/compile-pipeline.md) walks the four stages with the intermediate files.
- [CMake](../engineering/cmake.md) is how these units are listed without maintaining a second copy of the file names.
