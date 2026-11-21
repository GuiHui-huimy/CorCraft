# Arrays and Layout

Two dimensions are a convenience, not a type: the memory is one row after another, and every surprise in this chapter comes from someone forgetting the arithmetic that maps an index pair to an offset. I go from one dimension to two, then to the parameter that silently loses a dimension, then to the write one past the end, and only then to the fixes — variable-length arrays, a struct that carries the row width, and an in-place transpose that shows why the loop bounds matter.

An array is the simplest object in C and the one whose layout is most often assumed rather than measured. This chapter measures it: one dimension, then two, then the parameter form that silently loses the second dimension.

## 1. One dimension, contiguously

```c
int a[4] = {10, 20, 30, 40};
printf("%zu\n", sizeof a);              /* 16 */
printf("%td\n", &a[1] - &a[0]);         /* 1, in elements */
```

The four `int` objects are adjacent, with no gaps, because `sizeof(int)` is a multiple of `alignof(int)`. Adjacency is guaranteed by the standard, and it is what makes pointer arithmetic over an array meaningful: `a[i]` is defined as `*(a + i)`.

The one-past-the-end rule from the previous chapter has a second use here. `a + 4` is a valid pointer value, which is why the idiomatic loop uses `end` rather than an index:

```c
for (int *p = a; p != a + 4; ++p) { /* ... */ }
```

An index loop has the same semantics and one more opportunity to get the bound wrong. Neither form is faster after optimisation; the pointer form is chosen in this book because it makes the sentinel explicit.

## 2. Two dimensions are one dimension with arithmetic

`int m[3][4]` is not a matrix object. It is an array of three objects, each of which is an array of four `int`. The layout is row-major: the four elements of row 0, then the four of row 1, then row 2, with nothing between them.

```
offset of m[i][j] = i * 4 + j        (in elements)
address           = base + (i * 4 + j) * sizeof(int)
```

That formula is the entire content of two-dimensional indexing. It also explains why the column count must be known to compute an address: without it, `i * 4` has no meaning.

```c
/* LAB 04-01  arrays_layout/01_row_major_walk.c */
#include <stdio.h>

int main(void) {
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            printf("m[%d][%d]=%d  addr=%p\n", i, j, m[i][j], (void *)&m[i][j]);
        }
    }
    printf("row stride = %zu bytes\n", sizeof m[0]);
    return 0;
}
```

The printed addresses advance by 4 bytes within a row and by 12 bytes between rows — `sizeof m[0]`, not `sizeof(int)`. That number is the row stride, and it is the one to memorise: a row is an object, and the stride is its size.

## 3. The parameter that loses a dimension

```c
void correct(int m[][3], int rows);     /* column count preserved */
void broken(int m[2][3]);               /* rewritten to int (*)[3] ... still fine */
void wrong(int **m);                    /* a completely different object */
```

`void correct(int m[][3])` decays to `int (*m)[3]`: a pointer to an array of three `int`. Pointer arithmetic on it advances by 12 bytes, which is exactly one row. This is correct and it is why the column count appears in the parameter list.

`void wrong(int **m)` expects a pointer to a pointer. A `int[2][3]` array is not that: it is a contiguous block, and `m[1]` in the callee would read the second `int` of row 0 as if it were a pointer, then dereference it. The type system does not catch this; the resulting crash is several frames away from the call.

The rule: the inner dimension travels with the type, the outer dimension does not. Pass rows separately.

## 4. Out of bounds, and what actually happens

Array bounds are not checked. The standard defines the behaviour of an access outside the object as undefined, which in practice means one of four outcomes depending on what is adjacent in memory: the write lands in padding and nothing observable changes; it lands in another local and corrupts it; it lands on the return address and the function returns somewhere else; it lands on a guard page and the process dies.

```c
/* LAB 04-02  arrays_layout/02_bounds_probe.c */
int m[2][3];
m[0][4] = 99;    /* still inside the array object: it is m[1][1] */
m[2][0] = 99;    /* outside the object: undefined */
```

The first line is the one worth pausing on. `m[0][4]` is out of bounds for row 0 but inside the overall object, and row-major arithmetic makes it exactly `m[1][1]`. Nothing complains, because nothing is wrong from the memory model's point of view — only from the programmer's, who intended a row.

The second line is a genuine out-of-bounds write, and AddressSanitizer reports it with the object's bounds:

```
ERROR: AddressSanitizer: stack-buffer-overflow on address 0x...
WRITE of size 4 at 0x... thread T0
    #0 0x... in unsafe_write .../code/c/04_arrays_layout/02_bounds_probe.c:26
    #1 0x... in main         .../code/c/04_arrays_layout/02_bounds_probe.c:42
```

The first frame names the function that performed the write and the exact line — `a[LEN] = 0;` — while the second names the call that allowed it. That pair is why every memory chapter in this book runs under a sanitizer: the report converts an undefined outcome into a named one, with a line number you can open.

## 5. Variable-length arrays

C99 permits an array whose length is a run-time value:

```c
void scale(int n, double factor[n]) { /* ... */ }
```

The object lives in the caller's frame, so a large `n` is a stack overflow rather than a failed allocation. C11 makes the feature optional, MSVC does not implement it, and C++ does not have it at all. The book uses it exactly once, in a chapter that also shows the heap alternative, and the note on the page says so. For anything whose size is not known at compile time, the pattern that travels is `malloc` plus a length parameter.

## 6. Fixed-size rows as a type

The useful compromise between a raw pointer and a variable-length array is a pointer to a fixed-width row, because it keeps the stride in the type:

```c
void print_row(const double (*row)[3]) {
    printf("%f %f %f\n", (*row)[0], (*row)[1], (*row)[2]);
}
```

The biology section uses exactly this form for coordinates: a position is a three-element array, and a chain of positions is a contiguous block whose stride is `3 * sizeof(double)`. Passing `const double (*)[3]` means the callee cannot accidentally treat the block as a flat array of `double` and lose the grouping.

## 7. Transposing in place

A square matrix can be transposed without a second buffer by swapping above and below the diagonal:

```c
/* LAB 04-03  arrays_layout/03_transpose_inplace.c */
static void transpose(int n, int m[n][n]) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int t = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = t;
        }
    }
}
```

The loop starts at `j = i + 1`, not `j = 0`. Starting at zero swaps each pair twice and returns the original matrix — a defect that is invisible for symmetric input and obvious for any other. `03_transpose_inplace.c` asserts on an asymmetric case for that reason.

For a rectangular matrix the in-place swap is not possible, and the choice becomes row-major versus column-major traversal. The chapter on performance returns to this: walking a `1000 × 1000` matrix in the wrong order costs a cache miss per element, and the transpose itself is the standard fix.

## Continue

- [Structs and Padding](structs-and-padding.md) applies the same offset reasoning to named fields.
- [Performance](../engineering/performance.md) measures the traversal-order effect described above.
- [Geometry](../bio/geometry.md) uses fixed-width rows for coordinates, where the stride is the point.
