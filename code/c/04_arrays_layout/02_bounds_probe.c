/* LAB 04-02  arrays_layout/02_bounds_probe.c
 * Drills: the difference between readable and legal, and where the boundary is.
 * Build:  cmake --build build --target c_04_bounds
 *
 * Run without an argument for the checked version, which is what ctest runs.
 * Run with --unsafe to perform the out-of-bounds write and watch the sanitizer
 * report it. The two modes are in one file on purpose: the defect and its
 * detector should be one command apart.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <string.h>

#define LEN 8

static int checked_sum(const int a[LEN]) {
  int sum = 0;
  for (size_t i = 0; i < LEN; ++i) {
    sum += a[i];
  }
  return sum;
}

static void unsafe_write(int a[LEN]) {
  a[LEN] = 0; /* one past the end: undefined behaviour */
}

int main(int argc, char **argv) {
  int arr[LEN] = {1, 2, 3, 4, 5, 6, 7, 8};

  CRAFT_ASSERT(checked_sum(arr) == 36);
  printf("checked sum       = %d\n", checked_sum(arr));

  /* The boundary itself is legal to name; only the access is illegal. */
  int *one_past = arr + LEN;
  printf("one-past address  = %p (formable, not dereferenceable)\n",
         (void *)one_past);

  if (argc > 1 && strcmp(argv[1], "--unsafe") == 0) {
    printf("performing the out-of-bounds write (expect an ASan report)\n");
    unsafe_write(arr);
    printf("survived, which proves nothing: %d\n", arr[LEN - 1]);
  } else {
    printf("refused the write; rerun with --unsafe to see the report\n");
  }
  return 0;
}
