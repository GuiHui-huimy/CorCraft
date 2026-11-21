# Sorting and Cost

I put three algorithms in one table because the table is the argument: the measurements differ by more than the constants, and the shape of the input decides which row wins. Then I slow down and measure honestly, because a benchmark that ignores the timer's resolution reports zero. Quicksort's worst case is real, stability matters when a second key exists, and binary search is the reward for sorting. The Field Note is a benchmark that measured the wrong thing, which is the most common failure in this chapter.

Sorting is where complexity stops being a formula and becomes a measurement. Three algorithms on the same input differ by orders of magnitude, and the difference is visible with `clock()` in a single run — which is exactly what the original code in this book's archive did, comparing two selection-sort variants on 100,000 integers.

## 1. Three algorithms, one table

| Algorithm | Average | Worst | Stable | In place |
| --- | --- | --- | --- | --- |
| Selection sort | O(n²) | O(n²) | no | yes |
| Insertion sort | O(n²) | O(n²) | yes | yes |
| Quicksort | O(n log n) | O(n²) | no | yes (with a stack) |

The table is not the interesting part; the measurements are. Selection sort performs about `n²/2` comparisons regardless of the input, so a sorted array and a reversed array take the same time. Insertion sort performs `n²/4` comparisons on average and `n` on already-sorted input, which is why it is the right choice for small or nearly sorted arrays and why quicksort implementations switch to it below a threshold.

```c
/* LAB 14-01  sorting/01_sort_bench.c */
#include "craft_bench.h"
#include "craft_assert.h"

static void selection_sort(int *a, int n) {
    for (int i = 0; i < n - 1; ++i) {
        int min = i;
        for (int j = i + 1; j < n; ++j) {
            if (a[j] < a[min]) { min = j; }
        }
        if (min != i) { int t = a[i]; a[i] = a[min]; a[min] = t; }
    }
}
```

Two details in that body matter beyond the algorithm. The swap is outside the inner loop, so the inner loop is a pure comparison scan; the original code in the archive swapped inside, which turned each comparison into a conditional store and made the already-slow algorithm slower. And the `min != i` check avoids a self-swap, which is harmless for `int` and not harmless for a large struct.

## 2. Measuring honestly

Timing a sort requires three precautions, and the lab enforces all three.

```c
double t0 = craft_cpu_seconds();
selection_sort(copy, n);
double t1 = craft_cpu_seconds();
printf("selection n=%d  %.3f s\n", n, t1 - t0);
```

First, each algorithm runs on its own copy. Sorting in place and then reusing the array gives the second algorithm a sorted input, which is a different measurement.

Second, the timer wraps only the sort, not the allocation or the printing. I/O dominates everything at small `n`, and a benchmark that includes `printf` measures the terminal.

Third, the result is used. A sort whose output is never read can be removed by the optimiser entirely, and the measurement becomes a measurement of nothing. Printing a checksum of the array prevents it.

The numbers the lab expects on a typical laptop, for 20,000 integers:

```
selection  n=20000  1.6 s
insertion  n=20000  0.35 s
qsort      n=20000  0.003 s
```

The ratio between selection and `qsort` is roughly 500, and that ratio is the chapter's point. Complexity is not an abstraction when the constant is this large.

## 3. Quicksort's worst case is real

The textbook quicksort picks the last element as pivot. On an already-sorted array that pivot is the maximum every time, the partition produces one part of size `n − 1` and one of size zero, and the recursion depth becomes `n`. At `n = 100,000` the stack is gone before the sort finishes — the failure from chapter 2, reproduced by an algorithm that is correct on average.

Three repairs are standard, and the lab asks for two:

```c
static int median3(int a, int b, int c);          /* pick a pivot unlikely to be extreme */
static void sort_range(int *a, int lo, int hi, int depth);
```

The median-of-three pivot removes the sorted-input case. An explicit depth counter that switches to heapsort or insertion sort beyond `2 log₂ n` removes the adversarial case. Both are cheap and both are absent from the naive implementation, which is why library `qsort` implementations include them.

## 4. Stability, and when it matters

A stable sort preserves the relative order of elements that compare equal. `qsort` is not stable, and this is easy to forget because the comparator usually compares the whole element.

```c
struct hit { int score; int id; };
```

Sorting hits by `score` alone with an unstable sort can reorder two hits with the same score, which changes the output of any downstream step that assumes input order. The fix is either a stable sort (`mergesort`, or insertion sort for small arrays) or a comparator that breaks ties on `id`.

The C++ section's `std::stable_sort` exists for exactly this reason, and the difference in cost is worth measuring rather than assuming.

## 5. Searching a sorted array

Sorting is usually done to enable search. `bsearch` has the same comparator contract as `qsort`, and the same defect applies:

```c
int key = 42;
int *found = bsearch(&key, a, n, sizeof a[0], by_int);
if (found != NULL) { printf("%d\n", *found); }
```

Binary search is O(log n), and it fails silently on an unsorted array: it returns a wrong element or `NULL` depending on the path it took. The contract is on the caller, which is why the lab checks that the array is sorted before searching rather than trusting the caller's intent.

## 6. Field note: the benchmark that measured the wrong thing

An early version of this lab's benchmark sorted 100,000 random integers with two selection-sort variants and printed the elapsed time for each. The first variant swapped inside the inner loop; the second, correctly, found the minimum first. The times differed by roughly a factor of three on the machine it ran on, which was read as evidence that the second algorithm was algorithmically better.

It was not. Both are O(n²) with the same comparison count; the difference was the number of conditional stores, which is a constant-factor effect. The measurement was correct and the conclusion drawn from it was not, because the benchmark changed two things at once.

The lesson: a benchmark can only compare implementations that differ in one dimension. If the comparison count and the swap count both change, the timing tells you which is faster but not why, and the "why" is the part worth knowing.

## Continue

- [Linked Structures](linked-structures.md) sorts nothing but shares the pointer discipline.
- [Performance](../engineering/performance.md) turns this one-off timing into a repeatable measurement method.
- [STL Algorithms](../cpp/algorithms.md) is the same material with iterators and predicates.
