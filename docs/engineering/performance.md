# Performance

I refuse to start with a profiler, because most slow programs are slow for three reasons a timer can show you for free. I take the clock first and its resolution limits, then layout at small n, then the constant factor, then allocation, and only then the tool. Amdahl's law is one paragraph because it is one fraction. The field note is a benchmark that timed the optimiser away.

A performance claim without a measurement is a guess, and most of the guesses in this book turned out wrong when measured. This page is the measurement, kept deliberately small: one timer, one counter, and the three effects that account for almost every surprise in the labs.

## 1. The timer, and what it can and cannot see

```c
// code/include/craft_bench.h
static inline double craft_cpu_seconds(void) {
    return (double)clock() / (double)CLOCKS_PER_SEC;
}
```

`clock()` measures CPU time consumed by the process, not wall time. That is the right choice for a single-threaded lab: it excludes time the process spent descheduled, so a noisy machine does not change the number. It is the wrong choice for the parallel labs, where the whole point is to compare wall time across threads — those use `clock_gettime(CLOCK_MONOTONIC, ...)` and say so.

The resolution is typically 1 ms on Linux, which means a function that takes 10 µs cannot be timed by one call. The pattern is to time N repetitions and divide:

```c
double t0 = craft_cpu_seconds();
for (int i = 0; i < 1000; ++i) { work(); }
double t1 = craft_cpu_seconds();
printf("%.3f us/call\n", (t1 - t0) * 1e6 / 1000);
```

Three rules for a number worth quoting: the loop body must not be optimisable away (accumulate into a global and print it), the run must be long enough to exceed the resolution by two orders of magnitude, and the build must be `-O2` — a `-O0` measurement is a measurement of the debug build.

## 2. Effect one: layout beats algorithm at small n

The row-major walk lab compares two traversals of the same matrix:

```c
for (i) for (j) sum += a[i][j];      // row-major: sequential
for (j) for (i) sum += a[i][j];      // column-major: stride 400 bytes
```

Both are O(n²) and both perform the same number of additions. On a 2000×2000 `int` matrix the second is several times slower, entirely because each access misses the cache line that the next access in the loop needed. The mechanism:

- A cache line is 64 bytes = 16 `int`s.
- Row-major: 16 consecutive sums use one line, then one miss.
- Column-major: every sum uses a different line, so every sum misses.

At small n the matrix fits in L2 and the two loops are identical. The effect appears at a threshold, not gradually, which is why a benchmark at `n = 100` proves nothing and a benchmark at `n = 4000` proves everything.

## 3. Effect two: the constant factor is the code

The same algorithm written two ways can differ by 10× with no change in complexity. Three concrete cases from the book:

| Code | Operation | Why it differs |
| --- | --- | --- |
| `s += strlen(s)` in a loop | O(n²) hidden | `strlen` rescans the prefix each time |
| `std::endl` per line | one `write` per line | flush is a syscall |
| `printf("%d\n")` vs `fputs` | format parse per call | `printf` walks the format string |

The third one is the one people under-estimate: `printf` is roughly 10× slower than `fputs` for a fixed string, because it parses the format every call. In a loop that prints a million lines, the difference is a second.

## 4. Effect three: allocation is the hidden cost

`malloc` and `free` are cheap individually and expensive in aggregate. A program that allocates one small block per record pays for the bookkeeping, the fragmentation, and the cache misses that come from touching a new block each time.

The fix is structural rather than a faster allocator: allocate once, reuse. This is what `std::vector`'s capacity does, what the ring buffer lab does, and what the sequence reader does with `reserve`.

```c
// one allocation for N records, instead of N allocations
Record *records = malloc(N * sizeof *records);
```

The measurement that makes this believable: run the lab with `N` allocations and with one, and compare. On a typical machine the ratio is 3–10× for `N = 10^6`, and the larger the record, the worse the per-allocation version gets.

## 5. The tool, when the three effects are not enough

```bash
perf stat ./app                    # cycles, instructions, cache misses, branch misses
perf record -g ./app && perf report
valgrind --tool=callgrind ./app     # instruction counts per function, cross-platform
```

`perf stat` answers "is it cache, branch, or instruction count" in one line, and the cache-miss count is the fastest way to confirm effect one. `callgrind` is slower but deterministic and works without kernel permissions, which makes it the tool for a shared machine or a container.

Both are Linux tools. Route B and Route C readers get the same information from Visual Studio's profiler or from `valgrind`, and the chapter notes say so.

## 6. Amdahl's law, in one paragraph

If a fraction `p` of the work can be parallelised across `N` processors, the speedup is at most

\[
S(N) = \frac{1}{(1-p) + p/N}
\]

With `p = 0.9` and `N = 8`, the ceiling is about 4.7×, not 8×. With `p = 0.5`, it is 1.6×. This is the reason the parallel-scoring chapter measures the serial fraction before adding threads: a program with a 50% serial part cannot be saved by more cores, and the measurement is what tells you which part to attack.

## 7. Field note: the benchmark that measured nothing

A lab measured the cost of a function like this:

```c
clock_t t0 = clock();
compute(data);
clock_t t1 = clock();
printf("took %ld ticks\n", (long)(t1 - t0));
```

The output was `took 0 ticks` for every input. Two causes: `clock()`'s resolution is 1 ms and `compute` took microseconds; and at `-O2` with the result unused, the compiler was entitled to delete the call entirely.

The repair is both, not one: repeat enough times to exceed the resolution and consume the result so it cannot be removed. A benchmark that prints zero is not a fast function — it is a measurement that failed, and the number is not evidence of anything.

## 8. Practise

Run `code/c/14_sorting/01_sort_bench.c` at `-O0` and at `-O2` and explain the difference in the two lines it prints. Then attempt the exercise in `labs/c/04_arrays_layout/`: the row-major walk at `n = 4000` must be measurably faster than the column-major one, and a measurement inside the timer's noise must be rejected rather than reported.

## Continue

- [Sorting and Cost](../c/sorting-and-cost.md) is the algorithm side of the same measurements.
- [Parallel Scoring](../bio/parallel-scoring.md) applies Amdahl's law to a real workload.
- [Testing and Sanitizers](testing.md) explains why the timed build is a separate build.
