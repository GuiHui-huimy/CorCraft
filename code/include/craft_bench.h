#ifndef CRAFT_BENCH_H
#define CRAFT_BENCH_H

#include <time.h>

/*
 * craft_cpu_seconds() returns process CPU time in seconds.
 *
 * clock() is ISO C and needs no feature-test macro, which keeps every lab
 * buildable with -std=c17 and no -D flags. It measures CPU time rather than
 * wall time, so a benchmark of a single-threaded loop is directly comparable
 * across machines. For parallel labs the wall clock is taken separately in
 * docs/bio/parallel-scoring.md via a POSIX timer.
 */
static inline double craft_cpu_seconds(void) {
  return (double)clock() / (double)CLOCKS_PER_SEC;
}

#endif /* CRAFT_BENCH_H */
