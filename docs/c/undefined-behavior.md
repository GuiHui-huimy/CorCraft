# Undefined Behavior

I begin with a program the optimiser deletes, because the definition of undefined behaviour is easier to accept when you watch a loop vanish. Then five categories, then what each tool catches, then the shift rules that catch people who think they know the width. The section I care most about is the one on evidence: 'it works on my machine' is a sample from a set, not a proof. Reading a sanitiser report is the skill the chapter is actually teaching.

Undefined behaviour is not "a crash waiting to happen". It is a category the standard declines to describe, which means the compiler is entitled to assume it never occurs and to optimise as if it did not. That assumption is what makes UB more dangerous than a plain bug: a bug produces the wrong answer, while UB produces whatever the optimiser decided was consistent.

## 1. The optimisation that proves the point

```c
int always_true(int x) {
    return (x + 1) > x;
}
```

For every `int` except `INT_MAX`, the expression is true. For `INT_MAX`, `x + 1` overflows — undefined behaviour. The compiler therefore reasons: a program in which `x == INT_MAX` is a program with UB, and UB cannot occur in a correct program, so `x` is never `INT_MAX`, so the comparison is always true. GCC 11 at `-O2` compiles this function to `mov eax, 1; ret`. No comparison survives.

Nothing is broken in the compiler. The function's author wrote a program whose only guarantee comes from a condition that the language does not define, and the optimiser took the author at their word.

This is the difference between UB and implementation-defined behaviour. Reading an uninitialised `int` is UB: anything may happen. Right-shifting a negative `int` is implementation-defined: the standard says the implementation must document its choice, and GCC documents arithmetic shift. The first is a defect; the second is a portability question.

## 2. Five categories that cover most cases

| Category | Example | Detected by |
| --- | --- | --- |
| Uninitialised read | `int x; return x;` | MSan (not ASan), or by inspection |
| Out-of-bounds access | `a[5]` on a 4-element array | ASan |
| Signed overflow | `INT_MAX + 1` | UBSan |
| Invalid pointer arithmetic | `p + 2` past one-past-end | ASan, partially |
| Violated aliasing promise | `restrict` or strict-aliasing violation | Nothing reliable |

Casting a pointer to an incompatible type and dereferencing is a sixth, from the previous chapter, and it is deliberately not instrumented: the compiler cannot tell a legal `char *` access from an illegal `unsigned int *` access in general.

## 3. What each tool catches

```c
/* LAB 09-01  undefined_behavior/01_signed_overflow.c */
#include <stdio.h>
#include <limits.h>

int main(void) {
    int x = INT_MAX;
    printf("%d\n", x + 1);      /* signed overflow */
    printf("%d\n", -x - 1);     /* -INT_MIN, overflow again */
    return 0;
}
```

Compiled with `gcc -fsanitize=undefined`, both lines abort with a report naming the expression and the line:

```
runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
```

Compiled without the sanitiser, both print a value — usually `-2147483648` and `2147483647` — because x86 wraps. That wrapped value is not a guarantee; it is what this machine did this time. A different compiler version, or an inlined context, may produce something else.

AddressSanitizer and UBSan are complementary and are used together in this book's sanitizer target. ASan catches memory errors; UBSan catches arithmetic and shift errors. Neither catches an uninitialised read, which is why the debugging chapter mentions MemorySanitizer as the third tool and notes that it requires instrumenting every object in the program, including libraries.

## 4. Shift rules that catch people

```c
int x = 1 << 31;      /* UB in C: 2^31 is not representable in int */
int y = -8 >> 1;      /* implementation-defined, not UB */
int z = 1 << 32;      /* UB: shift count >= width of type */
```

Three different rules, three different outcomes. Shifting into the sign bit is UB for signed types; shifting a negative value right is implementation-defined; shifting by the width or more is UB for both signed and unsigned. The portable idiom for a bit mask at the top of a word is `1u << 31`, which is defined because `unsigned int` has 32 value bits and no sign bit to collide with.

## 5. Why "it works on my machine" is not evidence

A UB program has no defined behaviour, so every observation of it is a sample from an undefined set. The samples agree until a compiler version, an optimisation level, a platform, or an unrelated edit changes one of the inputs to the optimiser's reasoning.

The practical rule: when a defect disappears at `-O0` and appears at `-O2`, or appears on one machine and not another, the first hypothesis should be UB rather than a compiler bug. The second hypothesis is worth holding only after the code has been run under both sanitisers and read against the five categories above.

## 6. Reading a sanitiser report

```
ub_sweep.c:56:26: runtime error: shift exponent 32 is too large for 32-bit type 'unsigned int'
    #0 0x... in main .../code/c/09_undefined_behavior/02_ub_sweep.c:56
SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior ub_sweep.c:56:26
```

Four pieces of information, in order: the file and line, the category, the operation (`load`, `store`, `shift`), and the type. The category is the one to look up, because it determines the fix: a shift exponent at or past the type's width needs a check on the count (`shift < sizeof(unsigned int) * 8u`), a null load needs a check at the call site, and a signed overflow is a type that should have been unsigned or a bound that should have been checked before the arithmetic. All three appear in `02_ub_sweep.c`, one per branch, each paired with the guard that avoids it.

A report that names the same line twice usually means the expression appears in a macro expanded twice, which is a hint that the defect is in the macro's argument rather than in the function.

## 7. Field note: the loop that only ran on the developer's machine

A hand-written parser had a loop whose bound was computed as `end - start` and then used as an index into a table of `int`. On a machine where `start` preceded `end`, the subtraction was positive and the code worked. When a malformed record made `end == start`, the loop was skipped, and the code that was supposed to report the malformed record never ran — it was inside the loop.

The defect was not the subtraction but the absence of a branch for the empty case. `end - start` is a perfectly defined value; the assumption that it is at least one is what was undefined in the author's mind, not in the language. The fix was an explicit check before the loop, and the lesson is that most "UB" in real code is an unstated precondition rather than an exotic expression.

## Continue

- [Bit-Level Programming](bit-level-programming.md) shows the shifts that are defined and how to write them.
- [Error Handling](error-handling.md) replaces the missing precondition with a checked return code.
- [Debugging](../engineering/testing.md) lists which sanitiser catches which category.
