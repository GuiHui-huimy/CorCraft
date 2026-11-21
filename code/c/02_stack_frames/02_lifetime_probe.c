/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 02-02  stack_frames/02_lifetime_probe.c
 * Drills: block scope, storage duration, and the one function that must not
 *         return a pointer to its own local.
 * Build:  cmake --build build --target c_02_lifetime
 */
#include "craft_assert.h"

#include <stdio.h>

static int counter_static(void) {
  static int n = 0; /* initialised once, not per call */
  return ++n;
}

static int counter_auto(void) {
  int n = 0; /* initialised on every call */
  return ++n;
}

/* The defect this lab does not run: returning &local leaves a pointer to an
 * object whose storage no longer exists. The address is often still readable,
 * which is why the defect is silent — and why it is described here rather than
 * executed: at -O2 the compiler is entitled to make the returned value
 * meaningless, and a sanitizer may report it as a use-after-return.
 *
 *   static int *bad(void) { int local = 42; return &local; }   // never do this
 *
 * The repair is an output parameter, a caller-owned buffer, or a heap
 * allocation with a documented owner. */

static int sum_blocks(void) {
  int total = 0;
  {
    int inner = 10;
    total += inner;
  }
  {
    int inner = 20; /* a new object, same name, different storage */
    total += inner;
  }
  return total;
}

int main(void) {
  CRAFT_ASSERT(counter_static() == 1);
  CRAFT_ASSERT(counter_static() == 2); /* state survives the call */
  CRAFT_ASSERT(counter_auto() == 1);
  CRAFT_ASSERT(counter_auto() == 1); /* resets every call */

  CRAFT_ASSERT(sum_blocks() == 30);

  int *p = NULL;
  printf("a pointer outliving its object would be %p; this one is NULL\n",
         (void *)p);

  printf("static:  1, 2   auto: 1, 1   blocks: 30\n");
  return 0;
}
