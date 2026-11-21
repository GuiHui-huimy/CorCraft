/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 04-03  arrays_layout/03_transpose_inplace.c
 * Drills: in-place transpose of a square matrix, and why the rectangular case
 *         cannot be done in place.
 * Build:  cmake --build build --target c_04_transpose
 */
#include "craft_assert.h"

#include <stdio.h>

#define N 8

static void transpose(int m[N][N]) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = i + 1; j < N; ++j) { /* j starts at i+1: the diagonal is fixed */
      int tmp = m[i][j];
      m[i][j] = m[j][i];
      m[j][i] = tmp;
    }
  }
}

static void fill(int m[N][N]) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      m[i][j] = (int)(i * N + j);
    }
  }
}

int main(void) {
  int m[N][N];
  fill(m);

  CRAFT_ASSERT(m[2][5] == 2 * N + 5);
  transpose(m);
  CRAFT_ASSERT(m[5][2] == 2 * N + 5); /* the pair moved */
  CRAFT_ASSERT(m[3][3] == 3 * N + 3); /* the diagonal did not */

  printf("after transpose: m[5][2] = %d, m[3][3] = %d\n", m[5][2], m[3][3]);

  /* Writes to m[i][j] and m[j][i] touch two rows at once, so the same
   * row-major walk now misses on one of the two accesses. That cost is the
   * reason a blocked transpose exists for large matrices. */
  printf("in-place for square only: a 3x5 cannot be transposed without a buffer\n");
  return 0;
}
