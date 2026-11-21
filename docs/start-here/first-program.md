# First Program

The first program has two defects on purpose, because I want the reader to meet the compiler as a critic rather than as a gate. I compile it, ask for warnings, fix the program instead of the output, turn warnings into errors, look at what was produced, then ask the runtime as well. The rule at the end is the one I still follow: the warning is the bug, arriving early.

This page builds one small program four times, with increasingly strict compiler settings, and reads what comes back. The goal is not to print a greeting; it is to establish that the compiler is the first reviewer of every line you write, and that its complaints are evidence rather than noise.

## 1. A program with two defects

Create `warn.c` — the repository ships this exact file as `code/warn.c`, and the line numbers below refer to it:

```c
#include <stdio.h>

int main(void) {
    int unused_flag = 1;
    unsigned int total = 0;
    int delta = -5;

    total = delta;
    printf("total = %u\n", total);
    return 0;
}
```

Compile it the way most tutorials do, with no warning flags at all:

```bash
gcc warn.c -o warn
./warn
```

It prints `total = 4294967291` and exits `0`. Nothing failed. The program is nevertheless wrong in two places, and the compiler knew about both.

## 2. Ask for warnings

```bash
gcc -Wall -Wextra -Wconversion warn.c -o warn
```

Two messages appear, in this order:

```
warn.c:4:9: warning: unused variable 'unused_flag' [-Wunused-variable]
    4 |     int unused_flag = 1;
      |         ^~~~~~~~~~~
warn.c:8:11: warning: conversion to 'unsigned int' from 'int' may change the sign of the result [-Wsign-conversion]
    8 |     total = delta;
      |           ^~~~~
```

Read them as three separate pieces of information each: the location, the rule that fired, and the flag that controls it. The flag name is the important part, because it tells you what category of mistake you made and how to ask for the same check elsewhere.

`-Wunused-variable` is bookkeeping. A variable that is written and never read is either a leftover or a symptom that a computation was forgotten. In a 2,000-line file this warning is usually a real bug.

`-Wsign-conversion` is a semantic error wearing a warning's clothes. `delta` holds `-5`; assigning it to an `unsigned int` does not store `-5`, it stores the value that results from converting a negative signed integer to an unsigned type, which the standard defines as adding 2³². The printed `4294967291` is not a curiosity: it is the number that will be used in every subsequent arithmetic operation, including the bounds check that was supposed to reject it.

## 3. Fix the program, not the output

```c
#include <stdio.h>

int main(void) {
    unsigned int total = 0;
    int delta = -5;

    if (delta < 0) {
        fprintf(stderr, "delta must be non-negative, got %d\n", delta);
        return 1;
    }

    total = (unsigned int)delta;
    printf("total = %u\n", total);
    return 0;
}
```

Two changes, and only one of them is about types. The conversion is now explicit, because the cast is a place where the author asserts that the range has already been checked. And the range *is* checked, on the line above, so the assertion is true. A cast without a preceding check is the same defect with the evidence removed.

Rebuild with the same flags. There is no output, which is the only acceptable result:

```bash
gcc -Wall -Wextra -Wpedantic -Wconversion warn.c -o warn
echo $?
```

## 4. Turn warnings into errors

Warnings are easy to ignore when there are forty of them and the program still runs. The habit worth forming is to make ignoring impossible during development:

```bash
gcc -Wall -Wextra -Wpedantic -Wconversion -Werror warn.c -o warn
```

Now any warning aborts the build. Every lab in `code/` is compiled with that same warning set, and `labs/<track>/<stage>/check.sh` prints any warning it sees rather than aborting — the zero-warning rule is enforced by review, and the script keeps the evidence in front of the reader.

`-Werror` is a development setting, not a release setting. Third-party headers occasionally emit warnings you cannot fix without patching them; the usual compromise is to keep `-Werror` for your own translation units and to isolate the third-party include behind `-isystem`, which suppresses warnings from headers found there.

## 5. Look at what was produced

The compiler emitted an executable, not a script. Two commands make that visible:

```bash
file warn
nm warn | grep -E ' main| printf'
```

`file` reports an ELF 64-bit executable, dynamically linked. `nm` lists the symbol table: `main` is defined in this object (`T`), while `printf` is undefined (`U`) and will be resolved at load time by the dynamic linker. That `U` is the reason a C program needs the C library at runtime and a statically linked one does not.

This distinction becomes the subject of [Compile Pipeline](../engineering/compile-pipeline.md). For now it is enough to know that "compiled successfully" means four separate stages agreed, and the executable is the artefact of the last one.

## 6. Ask the runtime as well

The compiler checks what it can see. AddressSanitizer and UndefinedBehaviorSanitizer check what happens while the program runs. Add this line before the `printf`, deliberately out of bounds:

```c
int probe[4] = {1, 2, 3, 4};
probe[4] = 99;
```

Compile and run:

```bash
gcc -fsanitize=address,undefined -g warn.c -o warn_san
./warn_san
echo $?
```

The program now stops with a report naming `stack-buffer-overflow`, the address, and the line. Without the sanitizer, the same write usually succeeds silently, corrupting whatever the compiler happened to place after `probe`; the defect then surfaces somewhere unrelated, often in a different function.

## 7. The rule this page establishes

Every example in this book is compiled with `-Wall -Wextra -Wpedantic`, run under a sanitizer where memory is involved, and judged by exit status as well as output. If a program prints the right number but triggers a warning or a sanitizer report, it has not passed. That standard applies to the reader's own code from this page onward, which is why the lab check scripts enforce it automatically.

Two things to carry forward: a warning names the rule that fired, and the rule is usually a category of bug rather than a single line; and an explicit cast is a claim, so it must be preceded by the check that makes the claim true.

## Continue

- [Objects in Memory](../c/objects-in-memory.md) begins the C section with the question this page raised: how many bytes does each of these types actually occupy.
- [How to Use This Book](how-to-use.md) explains the numbering, the header comments and the `check.sh` protocol used by every example.
- [Compile Pipeline](../engineering/compile-pipeline.md) opens the four stages that produced `warn` and shows the intermediate files.
