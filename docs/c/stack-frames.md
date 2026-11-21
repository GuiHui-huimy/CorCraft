# Stack Frames

For a long time I treated the stack as an implementation detail I did not need to know. Then a function returned a pointer to its own local, and the program kept working for weeks. This page takes the frame apart in the order that makes that bug inevitable: the layout, then storage duration versus scope, then the stack's size, then where arguments actually live, and finally a debugger session that shows the frame you have been guessing at.

Every automatic object lives in a stack frame, and the frame is created and destroyed by a convention the compiler follows without telling you. Understanding that convention explains three things that otherwise look like folklore: why uninitialised locals hold garbage, why returning a pointer to a local is a defect rather than a mistake, and why a recursive function that works at depth 1,000 crashes at 100,000.

## 1. One frame, laid out

On x86-64 Linux, a call pushes a return address and then moves the stack pointer down. The callee's frame typically holds, in order: the saved frame pointer, the callee-saved registers it uses, its local variables, and the outgoing argument area. The exact arrangement is the compiler's business; what matters is that all of it is contiguous and all of it is reclaimed in one instruction when the function returns.

```c
/* LAB 02-01  stack_frames/01_frame_addresses.c */
#include <stdio.h>

static void callee(int depth) {
    int local = depth;
    printf("depth=%d  &local=%p\n", depth, (void *)&local);
    if (depth > 0) {
        callee(depth - 1);
    }
}

int main(void) {
    int outer = 0;
    printf("&outer=%p\n", (void *)&outer);
    callee(3);
    return 0;
}
```

The addresses printed by successive calls decrease, by roughly the size of one frame. That decrease is the whole story: the stack grows downward on this platform, and each call eats further into it. The numbers are not stable between runs — address-space randomisation moves the base — but the *ordering* is stable, and the ordering is what the chapter needs.

Two consequences are visible in the code. `local` is a distinct object in every invocation; there is no sharing, which is why recursion works at all. And `local` is uninitialised, so its content is whatever the previous frame left there. That is not a compiler bug: automatic objects have no defined initial value, and reading one before writing it is undefined behaviour, not "returns zero".

## 2. Storage duration is not scope

These two ideas are taught together and are not the same thing.

**Scope** is where a name is visible. **Storage duration** is how long the object behind the name exists. A name can go out of scope while its object lives on, and an object can outlive the block that created it — which is usually a defect.

```c
int *danger(void) {
    int local = 42;
    return &local;        /* the object dies at the closing brace */
}

int *safe(void) {
    static int preserved = 42;
    return &preserved;    /* static storage duration: lives until program exit */
}
```

The first function returns an address into a frame that no longer exists. Nothing detects it at compile time in C; GCC at `-O2` may warn with `-Wreturn-local-addr`, which is worth enabling. The second function is legal but has its own cost: `preserved` is one object shared by every caller, so it cannot be used for per-call state, and it is not thread-safe.

The distinction becomes urgent with `static` locals inside recursive functions. A `static` counter in a recursive function is one counter, not one per depth, which is rarely what the author intended.

## 3. The stack has a size

The stack is fast because it is small and pre-allocated: typically 8 MB on Linux (`ulimit -s`), 1 MB by default for a thread on Windows, and often 8 MB in a container. A frame of 1 KB therefore allows roughly 8,000 nested calls before the guard page is touched, and touching it produces a segmentation fault with no diagnostic beyond "stack smashing detected" or a bare `SIGSEGV`.

```c
/* LAB 02-02  stack_frames/02_lifetime_probe.c */
#include <stdio.h>

static int depth_of(int n) {
    char padding[512];              /* forces a frame large enough to matter */
    padding[0] = (char)n;
    return n == 0 ? 0 : 1 + depth_of(n - 1);
}

int main(void) {
    printf("%d\n", depth_of(8000));  /* adjust until it faults */
    return 0;
}
```

Three lessons come out of running it. The limit is reached at a depth that depends on the frame size, not on a constant. The failure is a signal, not an exception, so there is nothing to catch. And the fix is not "increase the stack" in most cases: it is either an explicit heap-allocated stack, or a loop, or an algorithm whose recursion depth is logarithmic rather than linear.

This is also why quicksort's worst case is a real problem and not a theoretical curiosity: an adversarial pivot choice on a sorted input produces recursion depth proportional to `n`, and at `n = 10⁶` the stack is gone before the first partition completes. Chapter 14 returns to this with an explicit depth limit and a switch to insertion sort on small ranges.

## 4. Where the arguments live

Arguments are not automatically in the frame. The System V AMD64 convention passes the first six integer arguments in registers (`rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`), and only the seventh onward go on the stack. That is why taking the address of a parameter gives you a copy: the compiler spilled the register into the frame so that the address exists at all.

```c
void takes_address(int value) {
    int *p = &value;      /* p points at a copy in this frame */
    *p = 7;               /* legal, and invisible to the caller */
}
```

Writing through that pointer does not change the caller's variable. C has no pass-by-reference; chapter 3's out-parameter pattern works because the caller passes an address explicitly, not because the callee received a reference.

## 5. Reading a frame with a debugger

The addresses above are enough to see the shape, but not the contents. `gdb` shows the frame as the compiler laid it out:

```
(gdb) break callee
(gdb) run
(gdb) info frame
(gdb) info locals
(gdb) p &local
(gdb) x/8wx $rsp
```

`info frame` reports the frame's base and the saved return address; `info locals` lists the objects the debugger knows about and their current values. Optimised builds collapse this: at `-O2` a local may exist only in a register, and `p &local` fails with "address requested for identifier which is not in memory". That is not a bug in the debugger; it is the compiler telling you the object has no address because it never needed one.

Debugging at `-O1 -g` is the compromise this book uses: enough optimisation to keep the code realistic, enough debug information to inspect frames. `-O0` is easier to read and misleading about timing, which matters from chapter 14 onward.

## Continue

- [Pointers](pointers.md) treats the addresses printed above as values in their own right.
- [Sorting and Cost](sorting-and-cost.md) is where stack depth becomes a design constraint, via recursion in the sort.
- [Error Handling](error-handling.md) shows what a stack overflow looks like in `gdb` and why it cannot be caught.
