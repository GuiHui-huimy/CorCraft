# Testing and Sanitizers

An assertion is a statement about one state; a test is a statement about a contract, and conflating them is why people say they test and still ship bugs. I start from that distinction, then `ctest` registration, then AddressSanitizer as the default build rather than an occasional flag, then Valgrind for the cases ASan cannot reach, then golden files with their escaping problem. The field note is a file that compiled and did nothing.

The repository has no test framework. It has assertions, `ctest`, and three sanitizers, which together cover the defect classes this book is about. This page is the manual for that combination.

## 1. Assertions are not tests, and the difference matters

```c
// code/include/craft_assert.h
#define CRAFT_ASSERT(cond) do {                                          \
    if (!(cond)) {                                                       \
        fprintf(stderr, "%s:%d: assertion failed: %s\n",                 \
                __FILE__, __LINE__, #cond);                              \
        abort();                                                         \
    }                                                                    \
} while (0)
```

`CRAFT_ASSERT` checks an invariant and stops the process with the file, line and expression. It is a debugging tool: it answers "did this hold", not "did the program do the right thing".

The distinction from a test is what happens on failure. An assertion that fires stops the run, so the first failure hides every subsequent one. A test suite reports each case and continues, so one run tells you how many things broke. For a lab with three checks, an assertion is enough; for the capstone with six, a suite is worth the extra file.

`assert` from `<assert.h>` is the standard version, and it has one trap: `NDEBUG` removes it entirely, so a release build silently drops every check. That is why `craft_assert.h` does not depend on `NDEBUG` — a check that disappears in the build you ship is worse than no check.

## 2. `ctest` runs what CMake registered

```bash
ctest --test-dir build --output-on-failure
```

Every `craft_lab()` call adds one test named after the target. A lab with no assertions still runs, and the test passes if the process exits zero — which is why a lab that is supposed to fail a check must be written to exit non-zero, not to print an error and continue.

## 3. AddressSanitizer is the default build

```bash
gcc -fsanitize=address,undefined -g main.c -o app
```

ASan wraps every allocation with red zones and checks every access against them. The report names the access type (`heap-buffer-overflow`, `heap-use-after-free`, `stack-buffer-overflow`), the address, the allocation site, and the free site. That is four facts in one report, which is why the labs build with it by default rather than adding it when something crashes.

Three reports worth recognising:

```text
heap-buffer-overflow on address ... READ of size 4
  #0 main in main.c:12      ← where the bad read happened
  ...
  0x... is located 0 bytes after 40-byte region   ← how far past the end
  allocated by thread T0 here:                    ← where the block came from
```

`stack-use-after-return` is the one ASan catches by default only with `-fsanitize-address-use-after-scope`, and it is the report for the function that returns a pointer to a local:

```c
int *bad(void) { int local = 42; return &local; }   // report on use, sometimes much later
```

UndefinedBehaviorSanitizer catches the things ASan cannot: signed overflow, shift by an out-of-range amount, misaligned access, null dereference. It does not wrap memory, so its reports are one line and its false-negative rate is higher.

LeakSanitizer runs at exit on Linux and reports every block still reachable — with a caveat: "still reachable" from a global is not the same as "definitely lost". The report distinguishes them, and only `definitely lost` is a defect.

## 4. Valgrind covers the case ASan cannot see

Valgrind does not instrument at compile time; it runs the binary on a synthetic CPU. That makes it slower (20–50×) and able to see things ASan cannot: uninitialised reads (`Conditional jump or move depends on uninitialised value`), and use of memory that ASan's red zones do not cover because the access is inside a valid block.

```bash
valgrind --leak-check=full --track-origins=yes ./app
```

`--track-origins=yes` is the flag that makes the uninitialised-value report useful: without it, the report says a branch depends on an uninitialised value; with it, the report says which allocation it came from.

The rule of thumb: ASan for a fast loop while developing, Valgrind once on a suspect, because Valgrind's uninitialised-value detection has no ASan equivalent.

## 5. Golden files for output

```bash
./app > out.txt
diff out.txt /tmp/baseline.txt        # the file you saved after reading the output once
```

`diff` on exact output is the cheapest possible regression test. Two conditions make it work: the program must write to `stdout` or a file the script controls, and the output must not contain timings, addresses, or floating-point values formatted differently on different platforms.

The second condition is the reason the capstone prints a `Fraction` rather than a `double`: `71/100` is byte-identical everywhere, while `0.71` may print as `0.710000` or `0.71` depending on the default precision. The capstone's own `check.sh` stops at exit status, and the program checks its report with `rfind` on the tab-separated prefix rather than by diffing a whole file — the header field is the part a reader is expected to edit, so a byte-for-byte comparison in the harness would fail on a legitimate change. The golden-file pattern above is what to use when the output is meant to be frozen.

For floating point that must be compared numerically, the rule is a relative tolerance:

```c
static int close_enough(double a, double b, double rel) {
    double d = a > b ? a - b : b - a;
    double m = (a > b ? a : b);
    return d <= rel * (m > 1.0 ? m : 1.0);
}
```

The `m > 1.0` guard is what makes the tolerance work near zero, where a relative comparison would demand impossible precision.

## 6. Field note: the file that compiled and did nothing

The archived C++ project contained `src/tesxt.cpp` — a typo in the name, and zero bytes of content. It appeared in the source list and compiled without complaint, because an empty translation unit is a legal translation unit.

The cost was invisible: a class declared in `tesxt.cpp`'s header, defined nowhere, produced an "undefined reference" at link time in a completely different file, and the search started from the link error rather than from the empty file.

Two habits prevent this. Compile with `-Wall -Wextra`, which does not warn about an empty file but does warn about the unused declarations; and let the build fail on emptiness — `-Werror=empty-body` covers the function-level case, and a `wc -c` check in a wrapper script covers the file-level one.

## 7. Practise

Build one of the C labs deliberately broken — introduce a one-past-the-end write — and read the ASan report until you can name the function, the line, the block size, and the distance past the end without looking anything up. Then attempt the exercise in `labs/bio/04_alignment/`: the full-table and two-row scores must agree exactly, and the self-alignment score for an eight-residue sequence must come out at 8 — the two assertions the reference program makes, which is why the run is also built under both sanitizers.

## Continue

- [Debugging](debugging.md) is what to do when the report is not enough.
- [Performance](performance.md) explains why the same lab is measured in a separate build.
- [How to Use This Book](../start-here/how-to-use.md) is the lab protocol these tools belong to.
