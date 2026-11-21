/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 04-01  arrays_layout/01_row_major_walk.c
 * Drills: row-major versus column-major traversal of the same matrix, and the
 *         cache effect that decides a benchmark.
 * Build:  cmake --build build --target c_04_rowmajor
 *
 * The matrix is static so the allocation cost stays out of the measurement.
 */
#include "craft_assert.h"
#include "craft_bench.h"

#include <stdio.h>

#define N 1024

static int matrix[N][N];
static volatile long sink; /* keeps the compiler from removing the loops */

static long walk_rows(void) {
  long sum = 0;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      sum += matrix[i][j];
    }
  }
  return sum;
}

static long walk_columns(void) {
  long sum = 0;
  for (size_t j = 0; j < N; ++j) {
    for (size_t i = 0; i < N; ++i) {
      sum += matrix[i][j];
    }
  }
  return sum;
}

int main(void) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      matrix[i][j] = (int)(i + j);
    }
  }

  double t0 = craft_cpu_seconds();
  long rows = walk_rows();
  double t1 = craft_cpu_seconds();
  double t2 = craft_cpu_seconds();
  long cols = walk_columns();
  double t3 = craft_cpu_seconds();
  sink = rows + cols;

  CRAFT_ASSERT(rows == cols); /* same answer, different cost */
  printf("row-major    %ld  %.4f s\n", rows, t1 - t0);
  printf("column-major %ld  %.4f s\n", cols, t3 - t2);
  printf("ratio        %.2fx\n", (t3 - t2) / (t1 - t0));

  /* The cache line is 64 bytes; an int is 4, so one line holds 16 of them.
   * Row-major uses one line for 16 sums; column-major uses a new line per sum. */
  printf("one cache line holds %zu ints\n", (size_t)64 / sizeof(int));
  return 0;
}
