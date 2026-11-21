/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 03-01  pointers/01_array_decay.c
 * Drills: array to pointer decay, sizeof at the two levels, what is lost.
 * Build:  cmake --build build --target c_03_decay
 */
#include "craft_assert.h"

#include <stdio.h>

/* The warning is suppressed because the array parameter shrinking to a pointer
 * is the lesson, not an oversight. A build with -Werror must still succeed. */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
#endif
static void takes_array(int arr[10]) {
  /* Inside the function the parameter is an int *, not an array. */
  printf("  in callee: sizeof(arr) = %zu (a pointer)\n", sizeof(arr));
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static size_t count_in(const int *p, size_t n) {
  (void)p; /* the length is the only information the callee has */
  return n;
}

int main(void) {
  int arr[10];
  for (int i = 0; i < 10; ++i) {
    arr[i] = i * i;
  }

  printf("in caller:   sizeof(arr)    = %zu\n", sizeof arr);
  printf("in caller:   sizeof(arr[0]) = %zu\n", sizeof arr[0]);
  printf("in caller:   element count = %zu\n", sizeof arr / sizeof arr[0]);
  takes_array(arr);

  int *p = arr; /* implicit decay: the name becomes &arr[0] */
  CRAFT_ASSERT(p == &arr[0]);
  CRAFT_ASSERT(*p == 0);
  CRAFT_ASSERT(*(p + 3) == 9);

  /* The size is not carried by the pointer; every function that takes a
   * pointer must be given the length separately. */
  CRAFT_ASSERT(count_in(arr, 10) == 10);
  printf("after decay: sizeof(p) = %zu, and the 10 is gone\n", sizeof p);
  return 0;
}
