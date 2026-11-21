/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 14-01  sorting/01_sort_bench.c
 * Drills: O(n log n) against O(n^2), and the cost of the comparison itself.
 * Build:  cmake --build build --target c_14_sort
 */
#include "craft_assert.h"
#include "craft_bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t N = 8000;

static int by_value(const void *pa, const void *pb) {
  int a = *(const int *)pa;
  int b = *(const int *)pb;
  return (a > b) - (a < b); /* the sign, never the difference */
}

/* Insertion sort: the O(n^2) baseline, fine for small n and hopeless here. */
static void insertion_sort(int *a, size_t n) {
  for (size_t i = 1; i < n; ++i) {
    int key = a[i];
    size_t j = i;
    while (j > 0 && a[j - 1] > key) {
      a[j] = a[j - 1];
      --j;
    }
    a[j] = key;
  }
}

static void fill(int *a, size_t n) {
  unsigned int seed = 12345u;
  for (size_t i = 0; i < n; ++i) {
    seed = seed * 1103515245u + 12345u; /* deterministic, no rand() */
    a[i] = (int)(seed % 100000u);
  }
}

int main(void) {
  int *a = (int *)malloc(N * sizeof *a);
  CRAFT_ASSERT(a != NULL);
  fill(a, N);

  int *b = (int *)malloc(N * sizeof *b);
  CRAFT_ASSERT(b != NULL);
  memcpy(b, a, N * sizeof *a);

  double t0 = craft_cpu_seconds();
  qsort(a, N, sizeof *a, by_value);
  double t1 = craft_cpu_seconds();

  insertion_sort(b, N);
  double t2 = craft_cpu_seconds();

  CRAFT_ASSERT(memcmp(a, b, N * sizeof *a) == 0); /* same order, different cost */
  for (size_t i = 1; i < N; ++i) {
    CRAFT_ASSERT(a[i - 1] <= a[i]);
  }

  printf("qsort        %.4f s\n", t1 - t0);
  printf("insertion    %.4f s\n", t2 - t1);
  printf("ratio        %.1fx\n", (t2 - t1) / (t1 - t0));
  printf("n = %zu, n^2 = %.0f\n", N, (double)N * (double)N);

  free(a);
  free(b);
  return 0;
}
