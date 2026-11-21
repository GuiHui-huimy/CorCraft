# Function Pointers and Callbacks

The type of a function is the part people skip, so I write it out — return type, parameter list, and the parentheses that make the pointer a pointer. Then the comparator, because `qsort` is where most readers meet a function pointer and where the subtraction overflow lives, then a dispatch table, then a callback that needs context. The cost section is short: an indirect call is a load and a branch, and the branch predictor decides whether you notice.

A function has an address. Storing that address in a variable turns a fixed call into a choice made at run time, which is how C expresses dispatch, comparators, and callbacks without any object system. Two thirds of the C++ section is an elaboration of this idea, so it is worth doing carefully in C first.

## 1. The type of a function

```c
int add(int a, int b);
int (*op)(int, int) = add;      /* the name decays to a pointer */
printf("%d\n", op(2, 3));
```

The declaration reads right to left: `op` is a pointer to a function taking two `int` and returning `int`. The parentheses around `*op` are required because `int *op(int, int)` would declare a function returning `int *`, which is a different thing — the ambiguity the previous chapter's table listed.

A `typedef` removes the noise and is what every real codebase uses:

```c
typedef int (*binary_op)(int, int);
binary_op table[] = { add, sub, mul };
```

## 2. Comparators, and the subtraction that overflows

`qsort` is the standard library's demonstration of a callback:

```c
/* LAB 11-01  callbacks/01_qsort_words.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int by_length(const void *a, const void *b) {
    const char *const *sa = a;
    const char *const *sb = b;
    size_t la = strlen(*sa), lb = strlen(*sb);
    return (la > lb) - (la < lb);      /* not la - lb */
}
```

The comparator returns a negative, zero, or positive value; `qsort` only uses the sign. Writing `la - lb` on `size_t` returns an unsigned value that is never negative after conversion to `int`, so the comparator only ever reports "greater" or "equal" — and the sort produces a confidently wrong order. The idiomatic `(a > b) - (a < b)` produces exactly −1, 0, or 1 with no overflow, for any ordered type.

The cast from `const void *` to `const char *const *` is the one place where a generic interface forces a type pun. It is legal because `qsort` passes pointers to the array elements, and the array elements are `const char *`. The two-level `const` is not decoration: it says the comparator may not modify the pointers it receives.

## 3. A dispatch table

A table of function pointers replaces a chain of `if` on a command string:

```c
/* LAB 11-02  callbacks/02_dispatch_table.c */
typedef int (*command_fn)(const char *arg);

struct command {
    const char *name;
    command_fn  fn;
    const char *help;
};

static const struct command commands[] = {
    {"len",   cmd_len,   "print the length of the argument"},
    {"rev",   cmd_rev,   "print the argument reversed"},
    {"count", cmd_count, "count vowels in the argument"},
};
```

Lookup is a linear scan over a table that is sorted by name at compile time — or a binary search when the table grows. The advantage over a switch is that the table is data: it can be printed as help, generated, or extended in one place without touching the dispatcher.

The `help` field is the part that makes this pattern worth teaching. A dispatch table that carries its own documentation can produce `--help` output without a second list to keep in sync, and the sync failure is otherwise inevitable.

## 4. Callbacks with context

A bare function pointer cannot carry state. The standard solution is a `void *` context passed alongside:

```c
typedef void (*visitor_fn)(void *context, const char *key, int value);

static void for_each(const struct map *m, visitor_fn visit, void *context) {
    for (size_t i = 0; i < m->len; ++i) {
        visit(context, m->items[i].key, m->items[i].value);
    }
}
```

The caller supplies a struct of its own and casts it back inside the callback. This is type-erasure in C: the library does not know what the context is, and the callback does. The alternative — a global variable — works for exactly one callback at a time and fails the moment two are registered, which is the defect that makes this pattern necessary rather than optional.

`void *` conversion to and from a function pointer is not guaranteed by ISO C, which permits the two to have different representations. POSIX guarantees it for object and function pointers on its platforms, and in practice every mainstream compiler allows it. Code that needs to be strictly conforming stores the function pointer in a `union` of the two types and documents the assumption.

## 5. What a function pointer costs

An indirect call cannot be inlined, so the compiler cannot see through it. On a hot loop this costs more than the call itself: the branch predictor has to guess a target that changes, and the call is not vectorised.

The measurement is worth doing once. A loop calling a function pointer that is never reassigned inside the loop is often devirtualised by the compiler — it can prove the target is constant — and then the cost disappears. A loop whose target changes per iteration cannot be, and the difference is typically a factor of two to five on a tight body.

This is the reason the C++ virtual-call chapter measures before and after devirtualisation, and the reason the dispatch table above is acceptable for commands (called once per user action) and unacceptable for an inner loop over atoms.

## 6. Field note: the comparator that sorted nothing

An early version of a student project passed `qsort` a comparator that returned `a->score - b->score`, where `score` was an `int` between 0 and 100. That is safe for those values. When the field was later widened to a full range and one record held a negative score, the subtraction overflowed on the negative end, and `qsort` produced a partially ordered array — sorted enough that a visual check passed, wrong enough that a binary search failed.

The repair was the comparison form above. The lesson is worth generalising: a comparator's contract is a sign, and a subtraction only supplies a sign when the range makes it safe. The contract does not promise a range, so the comparator cannot assume one.

## Continue

- [Files and Streams](files-and-streams.md) uses callbacks for a line-oriented reader.
- [Polymorphism](../cpp/polymorphism.md) replaces the dispatch table with a virtual table the compiler builds.
- [STL Algorithms](../cpp/algorithms.md) supplies comparators and predicates that do not require a context pointer.
