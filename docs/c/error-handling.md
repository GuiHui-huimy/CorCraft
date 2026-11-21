# Error Handling

C has no exceptions, so error handling is a design decision made in public: which layer reports, which layer translates, and which layer decides. I start with three layers rather than one, then `errno` and what it is not, then the `goto` cleanup pattern that makes partial construction safe. Return-code conventions come next, because a convention nobody wrote down is a convention nobody follows, and the `gdb` section exists because a failure that crashes and a failure that returns a wrong value are read differently.

C has no exceptions, so every failure travels as a value. The quality of a program is largely the quality of that travel: whether the failure is detectable, whether it carries a reason, whether the cleanup happens, and whether the caller is forced to acknowledge it.

## 1. Three layers, not one

Mixing these up is the most common error-handling defect:

| Layer | Mechanism | Cost when it fires | Correct use |
| --- | --- | --- | --- |
| Programmer error | `assert` | abort | An invariant that must hold if the code is correct |
| Expected failure | return code | a branch | A file that is missing, a parse that fails |
| Impossible state | `abort()` / `__builtin_unreachable` | abort | A switch arm that cannot be reached |

An `assert` on user input is a category error: it converts a malformed file into a crash, which is exactly what the security page argues against. A return code on an invariant is also a category error, because the branches pile up and are never tested.

```c
#include "craft_assert.h"

CRAFT_ASSERT(index >= 0 && index < n);   /* internal invariant */
```

In release builds `assert` may compile to nothing (if `NDEBUG` is defined), so an `assert` must never guard a side effect. The macro above is used in the labs precisely because it is always active: the checks in a lab are part of the lab.

## 2. `errno`, and what it is not

`errno` is a thread-local integer that library functions set on failure. It is not cleared on success, so it is only meaningful immediately after a call that reported failure.

```c
/* LAB 16-01  error_handling/01_errno_chain.c */
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    FILE *fp = fopen("missing.txt", "r");
    if (fp == NULL) {
        int saved = errno;                       /* copy before anything else */
        fprintf(stderr, "fopen failed: %s\n", strerror(saved));
        return saved;
    }
    fclose(fp);
    return 0;
}
```

The copy into `saved` is not paranoia. Any subsequent library call may overwrite `errno`, including `fprintf` itself, so the value must be read before the error message is built. This is the reason the diagnostic is captured first and formatted second.

`strerror` is not thread-safe on all platforms (`strerror_r` is the replacement), and it returns a pointer to a static buffer. In a single-threaded program that is fine; in the parallel lab it is not.

## 3. The `goto` cleanup pattern

The one place `goto` is standard practice in C is unwinding a partially constructed resource set:

```c
/* LAB 16-02  error_handling/02_cleanup_goto.c */
int load(const char *path, struct dataset *out) {
    int rc = -1;
    FILE *fp = NULL;
    char *buf = NULL;
    struct record *records = NULL;

    fp = fopen(path, "rb");
    if (fp == NULL) { goto done; }

    buf = craft_read_file(path, NULL);
    if (buf == NULL) { goto done; }

    records = parse_records(buf, &out->count);
    if (records == NULL) { goto done; }

    out->items = records;
    rc = 0;

done:
    free(records);      /* free(NULL) is a no-op */
    free(buf);
    if (fp != NULL) { fclose(fp); }
    return rc;
}
```

Three properties make this correct. Every acquisition is initialised to `NULL` or zero, so the cleanup path is valid at every point. The cleanup is written once, not at each failure point. And the success path falls through the same cleanup, so no resource is released in one path and forgotten in another.

The alternative — a cleanup block per failure — duplicates the release order in each branch, and the duplication is where the leaks live.

## 4. Return-code conventions

Two conventions are common, and mixing them in one codebase is a defect waiting:

```c
/* Convention A: 0 is success, negative is a category */
int rc = parse_file(path);        /* 0 ok, -1 io, -2 format */

/* Convention B: 0 is success, errno value on failure */
int rc = parse_file(path);        /* 0 ok, otherwise an errno value */
```

This book uses A inside the library (small negative categories, documented per function) and B at the boundary with the C library. A function that returns an `errno` value must document that the value is meaningful only when the return is nonzero, and must not confuse `-1` with `EPERM` on platforms where they differ.

Whatever the convention, it must be in the header comment. A function whose failure code is undocumented is a function the caller will check wrongly — usually by comparing against `0` and ignoring the value, which loses the reason and the log line that would have explained it.

## 5. Reading a failure in `gdb`

Errors that produce a wrong value are found by reading code; errors that crash are found by reading a core. A short session, on a two-function toy (`main.c` calling `load.c`) rather than on a file from `code/`, because the frame layout is the lesson and a toy keeps it short:

```
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x00005555555551a9 in load (path=0x0, out=0x7fffffffdcd0) at load.c:14
14          FILE *fp = fopen(path, "rb");
(gdb) bt
#0  load (path=0x0, out=...) at load.c:14
#1  main () at main.c:9
(gdb) p path
$1 = (const char *) 0x0
```

The frame line already answers the question: `path=0x0`. The call site passed `NULL`, and the function did not check. The fix is one line at the top of `load`, and the general rule is that every pointer parameter is either documented as non-null or checked.

A crash inside a library function with a plausible argument is usually an earlier corruption: `free` on an already-freed pointer, or a write past the end of a buffer. The sanitiser chapters exist because the crash site and the defect site are rarely the same.

## 6. Field note: the error that was reported as success

A batch script called a parsing function, checked `if (rc < 0)`, logged, and continued. The function returned `1` for a malformed record — a positive value, so the check did not fire, and the caller proceeded with a partially filled structure. Downstream, a missing residue was interpreted as a gap, and the geometry step produced a plausible wrong distance.

The contract had said "nonzero on failure". The caller had implemented "negative on failure". Both were internally consistent and mutually incompatible, which is the definition of an undocumented interface.

The repair was to make the contract explicit in the header — `Returns 0 on success, a positive error code otherwise` — and to change the check to `if (rc != 0)`. The lesson belongs here rather than in a review checklist: a return code is only as good as the sentence that documents it.

## Continue

- [Debugging with Sanitizers](../engineering/testing.md) covers the tools that find what the compiler cannot.
- [Linked Structures](linked-structures.md) applies the teardown rule to a graph.
- [Exceptions](../cpp/exceptions.md) is the C++ alternative, and it explains what is lost and gained by leaving return codes behind.
