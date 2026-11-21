# Pointers

A pointer is an address with a type attached, and I have made both halves wrong at different times. The type half is why `p + 1` moves by `sizeof(int)`; the address half is why an out-parameter can write back. I begin with the value and the type, then array decay because it explains the `sizeof` that lies at a call site, then out-parameters, then `const`, and I end with the three invalid pointers I have actually produced. If you can read a declaration aloud, the page has done its job.

A pointer is an integer that has been given two extra pieces of information: what type of object it points at, and permission to be dereferenced. The integer part is easy; almost every pointer defect comes from the second part being wrong — the pointer is valid in the arithmetic sense and invalid in the ownership sense.

## 1. The value and the type

```c
int x = 42;
int *p = &x;
printf("%p %zu\n", (void *)p, sizeof p);   /* an address, and 8 */
```

`p` holds the address of `x` and has type `int *`. The address alone would not be enough: `*(p + 1)` advances by `sizeof(int)`, not by one byte, precisely because the type is part of the pointer's value for arithmetic purposes.

```c
int a[5] = {10, 20, 30, 40, 50};
int *p = a;
printf("%d %d\n", *p, *(p + 2));   /* 10 30 */
printf("%td\n", (p + 2) - p);      /* 2, measured in elements */
```

Pointer subtraction yields a count of elements, not bytes, and its type is `ptrdiff_t`. Mixing the two units — advancing by bytes while subtracting in elements — is the arithmetic half of most off-by-one errors.

One boundary is worth memorising because it is legal and looks illegal: a pointer one past the last element may be formed and compared, but not dereferenced.

```c
int *end = a + 5;        /* legal: one past the end */
for (int *q = a; q != end; ++q) {
    printf("%d\n", *q);  /* the loop never reads *end */
}
```

`a + 5` is the canonical loop sentinel in C. `a + 6` is undefined behaviour even before dereferencing, because it is outside the array object plus one.

## 2. Decay, and the `sizeof` that lies

An array is not a pointer, but in almost every expression it becomes one. The conversion is called decay: the array's name turns into a pointer to its first element, and the size information disappears.

```c
/* LAB 03-01  pointers/01_array_decay.c */
#include <stdio.h>

static void by_array(int a[10]) {
    printf("in callee: %zu\n", sizeof a);   /* 8: it is a pointer */
}

int main(void) {
    int a[10];
    printf("in caller: %zu\n", sizeof a);   /* 40: it is an array */
    by_array(a);
    return 0;
}
```

The parameter `int a[10]` is rewritten by the compiler to `int *a`; the `10` is documentation, not a constraint. `-Wsizeof-array-argument` in Clang warns exactly on this pattern, and it is worth turning on. GCC's equivalent check arrives under `-Wsizeof-pointer-memaccess` for the `memset` case, which is the version that corrupts memory rather than merely printing a wrong number.

The practical rule: `sizeof` on a parameter never tells you the size of the caller's array. Pass the length explicitly, or pass a pointer to an array type — `void f(int (*a)[10])` — which preserves the width and is the one place where a pointer-to-array is genuinely useful.

## 3. Out-parameters, and why C has no references

C returns one value. When a function must produce two, the caller passes the address of a variable and the callee writes through it.

```c
/* LAB 03-02  pointers/02_out_param_swap.c */
static void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int main(void) {
    int x = 1, y = 2;
    swap(&x, &y);
    printf("%d %d\n", x, y);   /* 2 1 */
    return 0;
}
```

The `&` is the whole mechanism. Had `swap` taken `int a, int b`, the writes would have modified copies in its own frame and the caller would see nothing — the same phenomenon as writing through `&value` in the previous chapter. A pointer parameter is the caller saying "this object is mine to expose"; the callee's obligation is to check it is not null before writing, which is the contract chapter 16 formalises.

## 4. `const` is a position, not an adjective

Three placements, three meanings, and reading them from right to left is the reliable method:

```c
const int *p;         /* pointer to const int: *p = 1 is an error   */
int * const q = &x;   /* const pointer to int: q = &y is an error   */
const int * const r = &x;  /* neither the target nor the pointer moves */
```

The first is the common one in APIs: a function that promises not to modify the caller's data. The second is rare outside embedded code, because a pointer that cannot be reseated is usually better expressed as a reference in C++ or as an array.

`const` on a pointer parameter does not prevent the caller from modifying the object through another pointer. It is a contract between the function and its caller, enforced at compile time inside that function only.

## 5. The three invalid pointers

Every pointer is in one of four states: pointing at a valid object, one past the end, null, or invalid. Only the first may be dereferenced; the second may be compared; the last two are indistinguishable from valid addresses at run time on most systems.

**Null** is defined and checkable. `p = NULL` then `if (p)` is the standard contract.

**Dangling** points at an object that has been freed or whose frame has returned. It looks valid, and a read through it may return the old value for hours before the allocator reuses the block. AddressSanitizer catches the first use after free; nothing catches the use that happens before the reuse.

The "old value" part deserves a correction, because it explains why these bugs survive so long. glibc's tcache does not leave a freed small chunk untouched: it writes a `next` pointer and a `key` into the first sixteen bytes, so a dangling read of a small struct returns allocator bookkeeping rather than the fields you stored, and a dangling write corrupts the free list instead of the object. The corruption is invisible until the next `malloc` of that size class walks the list, which is why the crash usually lands in an allocation far from the defect.

**Wild** was never initialised. A local `int *p;` holds whatever the previous frame left there. The fix costs one line: `int *p = NULL;`.

The pattern that avoids all three is ownership discipline, which is what chapter 7 is about. A pointer that is always either owned by exactly one object or borrowed for a documented lifetime has no state in which it can be silently wrong.

## 6. Reading pointer types aloud

A declaration read right to left, with parentheses resolved first, disambiguates the forms that look alike:

| Declaration | Read as |
| --- | --- |
| `int *p[3]` | array of 3 pointers to int |
| `int (*p)[3]` | pointer to array of 3 int |
| `int *f(void)` | function returning pointer to int |
| `int (*f)(void)` | pointer to function returning int |
| `const char *const *argv` | pointer to const pointer to const char |

`int *p[3]` and `int (*p)[3]` differ by 16 bytes versus 8 in `sizeof`, and by completely different arithmetic. The chapter on function pointers uses the fourth form; the biology section uses the second when passing a fixed-width row of a coordinate matrix.

## Continue

- [Arrays and Layout](arrays-and-layout.md) follows the decay rule into two dimensions, where the column count becomes part of the type.
- [The Heap](heap.md) introduces pointers whose lifetime is not a frame.
- [Function Pointers](function-pointers.md) takes the `int (*f)(void)` form seriously.
