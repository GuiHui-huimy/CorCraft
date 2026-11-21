# The Heap

Heap defects share a property that makes them hard: nothing crashes at the moment of the mistake. I start with the two lifetimes a block has — allocated, then owned — because `malloc` gives you a block, `realloc` may move it, and the owner is whoever is willing to free it. The tools come after that, not before, because an ASan report only reads clearly when you already know which of the three errors you are looking at. Fragmentation is one section, not a chapter, because the fix is an arena, not a different allocator.

The stack gives every object a lifetime tied to a function call. The heap exists for the objects that outlive their frame, and it replaces the frame's automatic cleanup with an obligation: every block has exactly one owner, and that owner must free it exactly once.

That sentence is the whole chapter. Everything else is the consequence of violating it.

## 1. Two allocations, two lifetimes

```c
void stack_object(void)   { int a[4];       }   /* dies at the closing brace */
void heap_object(void)    { int *p = malloc(4 * sizeof *p); }  /* survives until free */
```

The stack object is reclaimed by a single instruction when the function returns. The heap block is reclaimed only when `free` is called on the exact pointer that `malloc` returned. If that call never happens, the block is lost to the process for its entire lifetime — and the process's lifetime is not the function's.

The heap also removes the size limit that the stack imposes. A frame of 8 MB is generous for a `100 × 100` matrix and hopeless for a `10,000 × 10,000` one, which is 800 MB. That is why every structure in the biology section whose count is not known at parse time is heap-allocated.

## 2. `malloc` returns a block, not an array

```c
int *p = malloc(n * sizeof *p);
if (p == NULL) { /* allocation failed: return an error, do not proceed */ }
```

Three details in that line are load-bearing.

`sizeof *p` rather than `sizeof(int)`: the idiom cannot drift from the pointer's type, and the drift is the defect in the previous chapter's field note.

The multiplication is not checked. `n * sizeof *p` overflows `size_t` when `n` exceeds `SIZE_MAX / sizeof *p`, and the result is a small allocation followed by writes past its end. The defensive form is a check before the call:

```c
if (n > SIZE_MAX / sizeof *p) { return NULL; }
```

`malloc` can return `NULL`, and every caller must handle it. A program that assumes success works until the machine is under memory pressure, which is exactly when the failure is least welcome.

`malloc` does not initialise. `calloc` zeroes, and is the right choice when a zero value is meaningless but harmless; `malloc` plus explicit initialisation is the right choice when every field will be written anyway.

## 3. `realloc` may move the block

```c
/* LAB 07-01  heap/01_growth_trace.c */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    size_t cap = 4, len = 0;
    int *data = malloc(cap * sizeof *data);
    if (data == NULL) { return 1; }

    for (int i = 0; i < 12; ++i) {
        if (len == cap) {
            size_t next = cap * 2;
            int *grown = realloc(data, next * sizeof *grown);
            if (grown == NULL) { free(data); return 1; }
            data = grown;
            cap = next;
            printf("realloc -> capacity %zu\n", cap);
        }
        data[len++] = i;
    }

    printf("final: len=%zu cap=%zu\n", len, cap);
    free(data);
    return 0;
}
```

`realloc` returns a pointer that may differ from the argument. When the allocator cannot extend the block in place, it copies the contents to a new block and frees the old one. The consequence is the most common heap defect in student code:

```c
data = realloc(data, next);   /* if realloc fails, data is leaked and NULL is lost */
```

The two-statement form above exists because `realloc` returning `NULL` leaves the original block intact. Assigning directly to `data` overwrites the only pointer to it with `NULL`, leaking the block and losing the handle at the same time. The temporary is not defensive style; it is the only correct form.

The growth trace also shows the amortised argument: doubling from 4 gives capacities 8, 16, 32, so twelve insertions trigger three reallocations rather than twelve. Growing by a constant instead makes the total cost quadratic, which is the subject of the dynamic-array discussion in the C++ section.

## 4. Ownership: one block, one owner

There is no reference counting in C and no way to ask the allocator who owns a block. Ownership is a convention the code must enforce, and the convention that works is the simplest one: exactly one pointer is responsible for freeing, every other pointer is a borrow whose lifetime is documented.

```c
struct buffer {
    char  *data;      /* owned */
    size_t len;
};

void buffer_free(struct buffer *b) {
    free(b->data);    /* the one place that frees it */
    b->data = NULL;   /* a second call is now harmless */
    b->len = 0;
}
```

Setting the pointer to `NULL` after freeing converts a double free from undefined behaviour into a defined no-op, because `free(NULL)` is specified to do nothing. It does not fix a dangling pointer held elsewhere — that copy still points at freed memory — but it removes the most common second free, which is the one through the same handle.

## 5. What the tools report

AddressSanitizer distinguishes three heap errors, and the distinction is worth reading carefully:

```
ERROR: AddressSanitizer: heap-buffer-overflow     # wrote past the block
ERROR: AddressSanitizer: heap-use-after-free      # read or wrote after free
ERROR: LeakSanitizer: detected memory leaks       # block never freed
```

The first two abort at the offending instruction. The third is reported at process exit, because "never freed" is only knowable once nothing else can free it. LeakSanitizer is part of ASan on Linux and runs by default; the report lists the allocation site and the size of each leaked block, which is enough to find the missing `free` without a debugger.

Valgrind reports the same three categories with more detail and roughly twenty times the slowdown. It is the fallback when ASan cannot run — inside some containers, or when the leak is in a library compiled without instrumentation.

## 6. Fragmentation, briefly

Freeing blocks in a different order than they were allocated leaves holes. A long-running process that allocates and frees blocks of varying size eventually has plenty of total free memory and no contiguous region large enough for the next request. This is external fragmentation, and no standard C facility addresses it.

The two practical mitigations are pool allocation (one large block carved into fixed-size slots, freed all at once) and arena reset (allocate everything, then discard the whole arena). Both appear in real parsers, including the one behind Protein Phaser: structures are parsed into a single arena and released in one call, so a 30,000-atom file costs one allocation and one free rather than 30,000 of each.

## 7. Field note: the leak that looked like a design

The `mocal.c` defect from the previous chapter has a second half that belongs here. After the ten inner blocks were allocated and the outer block freed, the program printed all ten strings correctly. It leaked every byte and looked, from the output, entirely healthy.

That is the shape of most leaks: no symptom, no crash, and a slow growth in resident memory that nobody measures until the process is killed by the OOM reaper. The only reliable detector is the tool, run routinely rather than when a problem is suspected.

The habit worth forming is smaller than the tool: every `malloc` in a function should have a matching `free` visible in the same function, or a comment naming the function that will free it. A block whose owner is not identifiable within the function that allocated it is a leak waiting for a branch that returns early.

## Continue

- [Qualifiers and Aliasing](qualifiers-and-aliasing.md) covers the compiler assumptions that make a use-after-free invisible at `-O0` and fatal at `-O2`.
- [Linked Structures](linked-structures.md) applies ownership to a graph of blocks rather than a single one.
- [RAII and Smart Pointers](../cpp/raii.md) is the C++ answer to the obligation this chapter describes.
