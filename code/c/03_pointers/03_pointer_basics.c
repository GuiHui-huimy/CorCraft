/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 03-03  pointers/03_pointer_basics.c
 * Drills: pointer arithmetic in units of the pointee, one-past-the-end,
 *         and the difference of two pointers.
 * Build:  cmake --build build --target c_03_basics
 */
#include "craft_assert.h"

#include <stddef.h>
#include <stdio.h>

int main(void) {
  int arr[5] = {10, 20, 30, 40, 50};
  int *begin = arr;
  int *end = arr + 5; /* one past the last element: legal to form, not to read */

  CRAFT_ASSERT(end - begin == 5);
  printf("end - begin     = %td\n", end - begin);

  /* p + 1 advances by sizeof(int), not by one byte. */
  int *p = begin;
  CRAFT_ASSERT(*(p + 1) == 20);
  printf("p+1 points at   = %d\n", *(p + 1));
  printf("byte distance   = %zu\n", (size_t)((char *)(p + 1) - (char *)p));

  /* Walking with a pointer rather than an index: the loop condition is the
   * pointer comparison, which is why one-past-the-end must be formable. */
  int sum = 0;
  for (int *it = begin; it != end; ++it) {
    sum += *it;
  }
  CRAFT_ASSERT(sum == 150);

  /* The defect: reading *end is out of bounds. The assertion is on the
   * pointer, never on the value. */
  CRAFT_ASSERT(end == &arr[5]);
  printf("sum             = %d\n", sum);

  ptrdiff_t d = &arr[3] - &arr[1];
  CRAFT_ASSERT(d == 2);
  printf("&arr[3]-&arr[1] = %td elements\n", d);
  return 0;
}
