/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 07-01  heap/01_growth_trace.c
 * Drills: realloc growth, the address that changes, and the capacity policy.
 * Build:  cmake --build build --target c_07_growth
 *
 * The trace is the point: the addresses are the evidence that realloc moves
 * the block, which is why every pointer into the old block becomes invalid.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
  size_t cap = 4;
  int *buf = (int *)malloc(cap * sizeof *buf);
  CRAFT_ASSERT(buf != NULL);
  printf("start   cap=%zu addr=%p\n", cap, (void *)buf);

  size_t used = 0;
  size_t moves = 0;
  for (int i = 0; i < 64; ++i) {
    if (used == cap) {
      size_t next = cap * 2;
      int *grown = (int *)realloc(buf, next * sizeof *buf);
      CRAFT_ASSERT(grown != NULL);
      if (grown != buf) {
        ++moves;
        printf("grow to cap=%-3zu addr=%p (moved)\n", next, (void *)grown);
      } else {
        printf("grow to cap=%-3zu addr=%p (in place)\n", next, (void *)grown);
      }
      buf = grown;
      cap = next;
    }
    buf[used] = i;
    ++used;
  }

  CRAFT_ASSERT(used == 64);
  CRAFT_ASSERT(buf[63] == 63);
  printf("final   used=%zu cap=%zu moves=%zu\n", used, cap, moves);

  /* The arithmetic behind the amortised claim: doubling means the total copy
   * work is at most twice the final size. */
  printf("total capacity copied: at most %zu ints\n", cap);
  free(buf);
  return 0;
}
