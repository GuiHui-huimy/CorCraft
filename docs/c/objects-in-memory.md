# Objects in Memory

I start with a value that looks like a typo: `sizeof('a')` is 4 on a C compiler and 1 in C++. That single number is the shortest proof that the object model, not the syntax, is what you are learning. From there I measure widths, then two's complement, then endianness, and only then alignment, because each section depends on the previous measurement rather than on a definition. By the end you should be able to predict a struct's size before printing it.

A C program does not manipulate values. It manipulates regions of memory that have been given a name, a width and an alignment, and the value is whatever those bytes happen to spell. Every later chapter depends on this picture: the stack is a region, the heap is a region, a struct is a region with a layout, a pointer is an integer holding an address inside some region.

The chapter opens with an anomaly that most readers have already seen without noticing.

## 1. `sizeof('a')` is not 1

```c
printf("%zu\n", sizeof('a'));
```

In C this prints `4`. In C++ the same expression prints `1`. Nothing is broken: in C, a character literal has type `int`, not `char`, because the language promotes it before any operator sees it. In C++, a character literal has type `char`. The two languages disagree about the width of the smallest thing you can write.

This is the first lesson of the section, and it is not about characters. It is about the difference between the *value* a literal denotes and the *object* the compiler creates for it. Asking "how big is `'a'`" is the wrong question; the right question is "what type did the compiler give this expression, and how many bytes does that type occupy on this target".

## 2. The widths, measured

The only trustworthy source for widths is the compiler on the target machine. Anything else is a table someone copied from a book written for a different platform.

```c
/* LAB 01-01  objects_in_memory/01_sizeof_table.c */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main(void) {
    printf("char        %zu\n", sizeof(char));
    printf("short       %zu\n", sizeof(short));
    printf("int         %zu\n", sizeof(int));
    printf("long        %zu\n", sizeof(long));
    printf("long long   %zu\n", sizeof(long long));
    printf("float       %zu\n", sizeof(float));
    printf("double      %zu\n", sizeof(double));
    printf("pointer     %zu\n", sizeof(void *));
    printf("size_t      %zu\n", sizeof(size_t));
    printf("'a'         %zu\n", sizeof('a'));
    return 0;
}
```

On x86-64 Linux with GCC the output is:

```
char        1
short       2
int         4
long        8
long long   8
float       4
double      8
pointer     8
size_t      8
'a'         4
```

On x86-64 Windows with MSVC the first four lines are identical and the fifth is the one that moves: `long` is 4 bytes, not 8. That difference is not a bug in either compiler. The C standard fixes only that `sizeof(char) == 1` and that `sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)`; everything else is the platform's choice. This is why the book uses `int32_t` and `int64_t` in the biology chapters, where a coordinate serialized on one machine is read on another.

`sizeof` is an operator, not a function call. Its result is `size_t`, an unsigned type, which is the reason `printf("%zu", ...)` is used above and `printf("%d")` would be a conversion warning — one the first chapter already taught you to read.

## 3. Two's complement, and the value that has no negation

Signed integers are stored in two's complement: the top bit carries a negative weight, and the rest carry positive weights. For a 32-bit `int`, the representable range is −2 147 483 648 to 2 147 483 647. The negative end has one more value than the positive end, and that asymmetry is the source of a defect that has shipped in production code more than once.

```c
int x = INT_MIN;
int y = -x;          /* undefined behaviour */
```

`-x` cannot be represented, because the positive counterpart of `INT_MIN` does not exist. The standard does not define what the machine should do, so the compiler may assume the negation never happens — and with optimisation enabled, it will assume exactly that. The signed overflow lab in chapter 9 demonstrates the consequence: the branch that was meant to guard the negation gets deleted, because a program in which `x == INT_MIN` is a program the compiler has decided cannot occur.

Unsigned types have no such hole. They wrap, by definition: `UINT_MAX + 1u == 0u`. That is why unsigned arithmetic is used for hashing and for bit manipulation, and why it is the wrong type for a loop bound that might go below zero.

## 4. Endianness

A multi-byte value has to be laid out in some order. Two orders dominate: least-significant byte first (little-endian, x86 and ARM in their usual modes), and most-significant first (big-endian, some network and embedded formats). The order is a property of the platform, and C gives you no portable way to ask — only a way to test at run time.

```c
/* LAB 01-02  objects_in_memory/02_endianness_probe.c */
union {
    uint32_t word;
    uint8_t  bytes[4];
} probe = { .word = 0x01020304u };

printf("%02x %02x %02x %02x\n",
       probe.bytes[0], probe.bytes[1], probe.bytes[2], probe.bytes[3]);
```

On a little-endian machine this prints `04 03 02 01`; on a big-endian machine, `01 02 03 04`. The `union` works because its members share the same storage: writing `word` and reading `bytes` is the standard-blessed way to inspect the representation without violating aliasing rules.

This matters the moment a file format is involved. A PDB file stores coordinates as text, so byte order is irrelevant there; a binary trajectory or a serialized matrix does not, and reading it with the wrong order produces numbers that are plausible and wrong by a factor of 16 million. Chapter 12 returns to this with `fread`.

## 5. Alignment, and why a struct is larger than its fields

Objects are not packed at arbitrary addresses. Each type has an alignment requirement — the addresses it may legally occupy must be multiples of that number — because the hardware fetches memory in aligned chunks, and some architectures fault on misaligned access. `alignof(int)` is 4 on x86-64, `alignof(double)` is 8.

Consequences follow immediately:

```c
struct pair { char c; int i; };
printf("%zu\n", sizeof(struct pair));   /* 8, not 5 */
```

The `char` occupies byte 0, three bytes of padding follow so that `i` starts at offset 4, and `i` occupies 4 through 7. The struct is 8 bytes. The padding is not waste in the sense of a leak — it is what makes the field reachable in one aligned load. Chapter 6 takes this apart with `offsetof` and shows how field order changes the total.

The rule worth remembering now: `sizeof` describes the object including its padding, and `memset` on a struct is only safe because the padding bytes are also part of the object.

## 6. What this buys

Three practical outcomes, before moving on.

A translation unit that prints the table above tells you more about the target than any specification. Keep the habit: when a width matters, print it.

`sizeof` on an expression reveals the type the compiler actually gave it, which is often not the type you wrote. `sizeof('a')` is the smallest example; `sizeof(arr)` versus `sizeof(ptr)` in chapter 4 is the one that breaks real code.

Representation is not value. A `float` holding `0.1` does not contain one tenth; it contains the nearest binary fraction, and the difference is why chapter 3 of the biology section spends a page on Kahan summation.

## Continue

- [Stack Frames](stack-frames.md) places these objects on the stack and gives each one a lifetime.
- [Structs and Padding](structs-and-padding.md) turns the alignment remark above into a measured layout table.
- [Undefined Behavior](undefined-behavior.md) explains why `-INT_MIN` is worse than a wrong answer.
