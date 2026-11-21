/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 09-02  undefined_behavior/02_ub_sweep.c
 * Drills: four undefined behaviours, each with the check that avoids it.
 * Build:  cmake --build build --target c_09_sweep
 *
 * Every case is written twice: once as the check, once as the operation it
 * guards. Only the checks run by default; --unsafe runs one operation.
 */
#include "craft_assert.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  int unsafe = (argc > 1 && strcmp(argv[1], "--unsafe") == 0);

  /* 1. Shift by a width that is too large. The check is on the count. */
  unsigned int value = 1u;
  unsigned int shift = 32u;
  if (shift < sizeof(unsigned int) * 8u) {
    printf("shift of %u allowed: %u\n", shift, value << shift);
  } else {
    printf("shift of %u refused: count must be < %zu\n", shift,
           sizeof(unsigned int) * 8u);
  }

  /* 2. Division by zero. Integer division has no defined result, unlike
   * floating point, which produces inf. */
  int divisor = 0;
  if (divisor != 0) {
    printf("division result %d\n", 10 / divisor);
  } else {
    printf("division by zero refused\n");
  }

  /* 3. Reading an uninitialised object. The check is that every buffer is
   * written before it is read; here the memset is the check. */
  int buf[8];
  memset(buf, 0, sizeof buf);
  CRAFT_ASSERT(buf[0] == 0);
  printf("buffer initialised before read: buf[0]=%d\n", buf[0]);

  /* 4. Overflow in the signed case, checked before it happens. */
  int count = INT_MAX;
  if (count > INT_MAX - 1) {
    printf("increment refused at INT_MAX\n");
  } else {
    printf("increment result %d\n", count + 1);
  }

  if (unsafe) {
    printf("performing 1u << 32 (expect a UBSan report)\n");
    volatile unsigned int bad = 1u;
    volatile unsigned int amount = 32u;
    unsigned int result = bad << amount; /* undefined */
    printf("result %u\n", result);
  }
  return 0;
}
