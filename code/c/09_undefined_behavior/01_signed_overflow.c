/* LAB 09-01  undefined_behavior/01_signed_overflow.c
 * Drills: signed overflow is undefined, unsigned wraps, and the compiler may
 *         use the undefined case to prove things that are not true.
 * Build:  cmake --build build --target c_09_overflow
 *
 * Default mode stays inside the range. --unsafe performs the overflow for
 * UBSan to report.
 */
#include "craft_assert.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Because signed overflow is undefined, the compiler may assume it never
 * happens, and that assumption lets it rewrite the comparison. */
static int is_positive(int x) { return x + 1 > x; }

int main(int argc, char **argv) {
  int unsafe = (argc > 1 && strcmp(argv[1], "--unsafe") == 0);

  printf("INT_MAX          = %d\n", INT_MAX);
  CRAFT_ASSERT(is_positive(5) == 1);
  printf("is_positive(5)   = 1, as arithmetic says\n");

  /* Unsigned arithmetic is defined to wrap modulo 2^n, which is why a hash or
   * a counter uses unsigned when wrapping is wanted. */
  unsigned int u = UINT_MAX;
  u = u + 1u;
  CRAFT_ASSERT(u == 0u);
  printf("UINT_MAX + 1     = %u (defined wrap)\n", u);

  if (unsafe) {
    printf("performing INT_MAX + 1 (expect a UBSan report)\n");
    int x = INT_MAX;
    int y = x + 1;
    printf("survived with %d: the value proves nothing\n", y);
  } else {
    /* The portable check: test before adding, never after. */
    int x = INT_MAX;
    if (x > INT_MAX - 1) {
      printf("refused INT_MAX + 1 by checking before the addition\n");
    }
  }

  /* The same idea for sizes: a multiplication that can overflow size_t must be
   * checked, because a wrapped size becomes a small buffer and a large write. */
  size_t a = 1000, b = 1000;
  if (a != 0 && b > SIZE_MAX / a) {
    printf("size_t multiplication would overflow; refused\n");
  } else {
    printf("1000 * 1000 = %zu, safe\n", a * b);
  }
  return 0;
}
