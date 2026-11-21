# Parallel Scoring

Parallelism is a measurement, not a mood, so I begin with the loop and then with the timer that lies — wall clock against CPU time, and why the first iteration is not the cost. Then the speedup arithmetic, then false sharing on one cache line, then reproducibility when the reduction order changes. The field note is a parallel program that reported a speedup of 1.0.

The last stage takes the scoring loop from the alignment chapter and distributes it. The loop is the ideal case — independent iterations, one reduction — and the page is mostly about what goes wrong anyway: the timer that lies, the reduction that changes answers, and the speedup that stops at 4× on an 8-core machine.

## 1. The loop

```c
/* LAB 09-01  bio_parallel/omp_scores.c */
double total = 0.0;
#pragma omp parallel for reduction(+:total) schedule(static)
for (size_t i = 0; i < n_ligands; ++i) {
    double s = score_ligand(ligands[i], &protein);
    total += s;
}
```

Three clauses, three decisions.

`parallel for` requires that the iterations be independent — no iteration writes what another reads. The scoring loop satisfies this because each ligand is scored against a read-only protein.

`reduction(+:total)` gives each thread a private `total`, initialised to the identity (0), combined at the end. The combination order is unspecified, which is the reproducibility problem below.

`schedule(static)` divides the iteration space into equal contiguous chunks. For a uniform workload that is optimal: no scheduling overhead, predictable locality. For a workload where some ligands take ten times longer than others, `static` leaves one thread doing the long ones while the others idle, and `dynamic` or `guided` is the right choice at the cost of a lock per chunk.

```c
schedule(dynamic, 64)   // fetch 64 iterations at a time
schedule(guided)        // chunks shrink as the loop progresses
```

The rule: measure the variance in per-iteration cost first. If the ratio of slowest to fastest is under 2, `static` wins. Above that, the load imbalance costs more than the scheduling.

## 2. The timer that lies

```c
clock_t t0 = clock();
#pragma omp parallel for
for (...) { ... }
clock_t t1 = clock();
printf("%.2f s\n", (double)(t1 - t0) / CLOCKS_PER_SEC);   /* wrong for parallel */
```

`clock()` returns CPU time summed over all threads on Linux (and on Windows it returns wall time, which makes the error platform-dependent and therefore worse). On an 8-thread run that finishes in 1 second, `clock()` reports about 8 seconds, so the computed "speedup" is negative or nonsense.

The correct timer for wall time:

```c
#include <time.h>
struct timespec a, b;
clock_gettime(CLOCK_MONOTONIC, &a);
/* work */
clock_gettime(CLOCK_MONOTONIC, &b);
double secs = (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) * 1e-9;
```

`CLOCK_MONOTONIC` is not affected by wall-clock adjustments and has nanosecond resolution. Route B uses `QueryPerformanceCounter` or `std::chrono::steady_clock`, which is the portable form and is what the C++ version of the lab uses.

## 3. What the speedup actually is

A worked example for `1A3N` against 1000 ligands on a 4-core machine. The numbers are chosen to show the arithmetic and are consistent with the serial fraction used below; they are not copied from a recorded run, so when you run the lab your own numbers will land near these and not on them:

| Threads | Wall time | Speedup | Efficiency |
| --- | --- | --- | --- |
| 1 | 4.00 s | 1.00× | 100% |
| 2 | 2.20 s | 1.82× | 91% |
| 4 | 1.30 s | 3.08× | 77% |
| 8 | 1.29 s | 3.10× | 39% |

The 8-thread row on a 4-core machine is the oversubscription case: the OS time-slices 8 threads onto 4 cores, so the row is roughly the 4-thread row again. The efficiency column is the honest number — 3.1× on 8 threads is 39% efficiency, not 3.1× "scaling".

The 4-thread row is below 4× because of the serial fraction. With \(p = 0.9\), Amdahl's law gives a ceiling of 3.08× at 4 threads, which is where the table's value sits; a real run lands near it, and the difference is noise rather than a contradiction.

The lab requires the reader to report all four numbers and the serial fraction, estimated by running with 1 thread and timing a serial-only section. A speedup number without its thread count and efficiency is not a measurement.

## 4. False sharing

```c
double partial[NTHREADS];                    /* each thread writes partial[id] */
#pragma omp parallel for
for (...) { partial[omp_get_thread_num()] += ...; }
```

Eight `double`s are 64 bytes: one cache line. Every thread writes to the same line, so every write invalidates every other core's copy, and the line ping-pongs between caches. The loop is correct and can be slower than the serial version.

The fix is padding:

```c
struct padded { double v; char pad[56]; };
struct padded partial[NTHREADS];
```

`reduction` avoids this problem because each thread's private copy lives on its own stack. A hand-written per-thread accumulator without padding reintroduces it, which is why the lab's second exercise asks for the hand-written version and then measures the padded and unpadded variants.

## 5. Reproducibility

Floating-point addition is not associative, so a `reduction` over 8 threads produces a different last few digits than the serial loop:

```c
/* serial: 0.1234567890123456 */
/* 8 threads: 0.1234567890123449 */
```

The difference is \(10^{-14}\), and it does not matter for a score. It matters when a result is compared across a serial and a parallel run, which is why the labs that print a distance or an energy compare with a tolerance rather than byte-for-byte, and why the capstone prints a `Fraction` instead of a `double`.

For an exactly reproducible parallel sum, the reduction must be deterministic: partition the index range explicitly into fixed chunks, accumulate each chunk in a fixed order, then combine the chunks in a fixed order. `schedule(static)` plus a manual array of partials does that, and it is slower than `reduction` by a few percent. The lab offers both and states the trade.

## 6. Field note: the parallel program that reported no speedup

A course implementation added `#pragma omp parallel for` and measured with `clock()`, reported a 0.9× speedup on 8 threads, and concluded that "the machine does not support parallelism". The measurement was wrong twice: `clock()` summed the threads' CPU time, and the loop was over an array of 10 elements, so the thread launch overhead exceeded the work.

The repairs are the two rules of this page: time the wall clock, and size the problem so that each thread gets enough work to amortise the launch. `omp_get_wtime()` exists for exactly this and returns wall time in seconds.

The second defect — 10 iterations across 8 threads — produced a measurement that could not have shown a speedup under any timer. A benchmark that is not sized to its mechanism is not a benchmark.

## 7. Practise

Build `code/bio/09_parallel/omp_scores.c` with `-fopenmp` and run it at 1, 2, 4 and 8 threads, recording wall time, speedup and efficiency. Then run `bash labs/bio/09_parallel/check.sh`: it builds the same target with whatever OpenMP the compiler offers, runs it, and asserts that the reduced result matches the serial one to a tolerance — the order-independence claim, which is the part that must hold. The speedup and the efficiency are your measurements, taken with the commands above.

## Continue

- [Performance](../engineering/performance.md) is the Amdahl discussion this page applies.
- [Alignment](alignment.md) is the scoring function being parallelised.
- [Contacts](contacts.md) is the other loop that scales the same way.
