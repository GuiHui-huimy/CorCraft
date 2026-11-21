/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 02-03  stack_frames/03_recursion_depth.c
 * Drills: recursion cost, and the depth at which the stack is the limit.
 * Build:  cmake --build build --target c_02_depth
 *
 * The recursion is capped at a safe depth: running until the process dies is
 * the lecture, not the lab. The measured depth and the timing are the numbers.
 */
#include "craft_assert.h"
#include "craft_bench.h"

#include <stdio.h>

/* Eight ints of locals per frame, so the frame is large enough to see. */
#define PAD 8

static int descend(int limit, int depth) {
  int pad[PAD] = {0};
  pad[0] = depth;
  if (depth >= limit) {
    return pad[0];
  }
  int below = descend(limit, depth + 1);
  return pad[0] > below ? pad[0] : below;
}

int main(void) {
  const int limit = 1000;

  double t0 = craft_cpu_seconds();
  int reached = descend(limit, 0);
  double t1 = craft_cpu_seconds();
  CRAFT_ASSERT(reached == limit);
  printf("depth %d reached in %.6f s\n", reached, t1 - t0);

  /* An iterative version of the same count: the comparison is the frame cost. */
  t0 = craft_cpu_seconds();
  int count = 0;
  for (int i = 0; i <= limit; ++i) {
    int pad[PAD] = {0};
    pad[0] = i;
    count += pad[0];
  }
  t1 = craft_cpu_seconds();
  CRAFT_ASSERT(count == limit * (limit + 1) / 2);
  printf("iteration over %d frames: %.6f s\n", count, t1 - t0);

  printf("one frame holds %d ints plus the return address and saved registers\n",
         PAD);
  return 0;
}
