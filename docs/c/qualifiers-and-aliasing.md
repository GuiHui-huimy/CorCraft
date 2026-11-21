# Qualifiers and Aliasing

`const` is a position, not an adjective, and I have put it in the wrong position often enough to write the section slowly. `volatile` is about observation rather than synchronisation, `restrict` is a promise I can break, and strict aliasing is the rule with no keyword at all. I keep the last two sections for `-O2` and a Field Note, because that is where the difference between `-O0` and `-O2` stops being academic and starts being a segmentation fault.

Three keywords — `const`, `volatile`, `restrict` — and one rule that has no keyword at all: strict aliasing. The keywords are promises to the compiler, and the rule is the promise the compiler makes back. Misunderstanding either produces code that works at `-O0` and fails at `-O2`, which is the worst kind of defect because it is attributed to the optimiser rather than to the code.

## 1. `const` objects may live in read-only memory

`const` on a pointer (previous chapter) restricts the pointer. `const` on an object restricts the object:

```c
const int limit = 100;
int *p = (int *)&limit;   /* compiles, with a cast */
*p = 42;                  /* undefined behaviour */
```

Nothing in the language prevents the cast, and nothing at run time necessarily prevents the write — but on most platforms a `const` object with static storage duration is placed in a read-only segment, so the write faults. The diagnostic is a segmentation fault at an instruction that looks like an ordinary store.

The lesson is that `const` is not advisory. Casting it away and writing is undefined behaviour, and the failure mode is platform-dependent: a fault on Linux, a silent success on a platform that puts constants in writable memory, and a wrong value in either case when the optimiser has already substituted the constant into the code.

## 2. `volatile` is about observation, not synchronisation

`volatile` tells the compiler that the object may change without any code in this translation unit writing it, so every access must actually happen, in source order, and must not be cached in a register.

Three uses are correct:

```c
volatile uint32_t *const status = (volatile uint32_t *)0x40021000;  /* memory-mapped register */
volatile sig_atomic_t stop_flag;                                    /* written by a signal handler */
volatile int after_longjmp;                                         /* setjmp/longjmp clobber rule */
```

One use is wrong and extremely common: using `volatile` as a substitute for a mutex or an atomic.

```c
volatile int ready = 0;   /* NOT a synchronisation primitive */
```

`volatile` guarantees that the access happens; it does not guarantee atomicity, and it does not establish ordering with respect to other memory operations on another thread. On x86 it may appear to work because the hardware is strongly ordered; on ARM it fails intermittently. The replacement is `<stdatomic.h>`:

```c
atomic_int ready = 0;
atomic_store_explicit(&ready, 1, memory_order_release);
```

The two keywords are unrelated despite the similar spelling. `const` is about permission to write; `volatile` is about the obligation to read.

## 3. `restrict` is a promise about aliasing

```c
void scale(size_t n, double *restrict dst, const double *restrict src);
```

`restrict` asserts that, for the duration of the call, the object accessed through `dst` is not accessed through any other pointer. The compiler may then reorder loads and stores freely, because it no longer has to consider that writing through `dst` might change what `src` reads.

The promise is on the caller, and breaking it is undefined behaviour rather than a wrong result in some defined sense. The standard example where the promise fails is `memcpy` with overlapping regions — which is exactly why `memmove` exists as a separate function with no `restrict`. A function that internally calls itself with overlapping arguments has broken the promise even though the caller's pointers were distinct.

`restrict` is the one qualifier that can be added to an existing correct function without changing its semantics, and the vectorisation chapter uses it to make a loop optimisable. `-Wrestrict` in GCC diagnoses the overlap cases it can see at compile time, and it is worth enabling for any code that uses the keyword.

## 4. Strict aliasing: the rule with no keyword

The compiler assumes that two pointers of incompatible types do not point at the same object, with a small list of exceptions. Under that assumption it may keep a value in a register across a store through the other pointer.

```c
/* LAB 08-01  qualifiers/01_alias_violation.c */
#include <stdio.h>

static int reinterpret(float f) {
    unsigned int bits = *(unsigned int *)&f;   /* undefined behaviour */
    return (int)(bits >> 23 & 0xFF) - 127;
}

int main(void) {
    printf("%d\n", reinterpret(1.0f));
    return 0;
}
```

At `-O0` this usually prints `0`, which is the correct exponent of 1.0. At `-O2` it may print a different value, because the compiler is entitled to assume the `unsigned int *` does not alias the `float` object and may propagate the known value of `f` instead of reloading it.

Three legal ways exist to do the same job, and they differ in cost:

```c
union { float f; unsigned int u; } pun = { .f = 1.0f };   /* legal in C */
unsigned int bits;
memcpy(&bits, &f, sizeof bits);                            /* legal, optimised away */
```

The union is the standard-blessed reinterpretation in C (though not in C++ in the same form, a fact the C++ section notes). `memcpy` is legal everywhere and compiles to a single move when the sizes are known. Both are correct; the pointer cast is not.

The exception list matters for the biology code: `char *` may alias any object, which is why `memcpy` and `memset` are permitted to operate on any type. A parser that reads a `float` through a `char *` is therefore legal, and one that reads it through an `unsigned int *` is not.

## 5. Why this shows up at `-O2` and not at `-O0`

Optimisation levels change which assumptions the compiler acts on, not which rules exist. `-O0` keeps every variable in memory and reloads after every store, so a violation happens to produce the expected value. `-O2` keeps values in registers, and the violation becomes visible.

This is the reason the book compiles examples at `-O1` with `-g` by default and re-runs the aliasing labs at `-O2` deliberately. A defect that disappears when you lower the optimisation level is not a defect in the optimiser.

## 6. Field note: the constant that could not be modified

An early version of a configuration helper declared its table as `static const` and then, in an "override" path, cast away the constness and wrote into it. On the developer's machine the write succeeded, because the table had been placed in writable memory by that build. On a distribution build with `-O2`, the same code faulted at start-up, and the crash was attributed to a compiler upgrade that had changed the section layout.

The fix was not to change the section layout back. It was to keep two tables — the immutable default and a mutable working copy — which is what the code should have done from the beginning. The general lesson: a cast that removes `const` is a statement that the object was never really constant, and if that statement is false, no compiler flag can rescue it.

## Continue

- [Undefined Behavior](undefined-behavior.md) classifies the failures this chapter produces.
- [Performance](../engineering/performance.md) shows where `restrict` actually pays.
- [Geometry](../bio/geometry.md) uses `memcpy` for coordinate reinterpretation rather than pointer casts.
