/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 03-02  pointers/02_out_param_swap.c
 * Drills: passing by pointer to modify the caller's object, and the reason a
 *         function cannot swap through by-value parameters.
 * Build:  cmake --build build --target c_03_swap
 */
#include "craft_assert.h"

#include <stdio.h>

/* Correct: both parameters name the caller's objects. */
static void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

/* Wrong on purpose: the copies are swapped, the originals are untouched. */
static void swap_by_value(int a, int b) {
  int tmp = a;
  a = b;
  b = tmp;
  (void)a;
  (void)b;
}

/* The same defect in a shape that looks safer: the pointer is const, so the
 * pointee cannot be modified at all. */
static void swap_through_const(const int *a, const int *b) {
  (void)a;
  (void)b;
}

int main(void) {
  int x = 1, y = 2;

  swap(&x, &y);
  CRAFT_ASSERT(x == 2);
  CRAFT_ASSERT(y == 1);
  printf("after swap:            x=%d y=%d\n", x, y);

  swap_by_value(x, y);
  CRAFT_ASSERT(x == 2);
  CRAFT_ASSERT(y == 1);
  printf("after swap_by_value:   x=%d y=%d  (unchanged)\n", x, y);

  swap_through_const(&x, &y);
  printf("after swap_through_const: x=%d y=%d\n", x, y);

  /* NULL is the other failure mode: a pointer parameter that may be absent
   * must be checked before it is dereferenced. */
  int *nothing = NULL;
  if (nothing == NULL) {
    printf("null pointer refused before dereference\n");
  }
  return 0;
}
