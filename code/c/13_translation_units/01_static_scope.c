/* LAB 13-01  translation_units/01_static_scope.c
 * Drills: internal versus external linkage, the header that declares, and the
 *         one-definition rule this file can only demonstrate from one side.
 * Build:  cmake --build build --target c_13_static
 */
#include "craft_assert.h"

#include <stdio.h>

/* Internal linkage: invisible to every other translation unit. Two units may
 * each have their own helper() with no conflict, because neither exports it. */
static int helper(int x) { return x * 2; }

/* External linkage: visible to the whole program, so exactly one definition
 * may exist anywhere. If a second unit defined it, the link would fail. */
int exported_helper(int x) { return x + 1; }

/* A static object at file scope is one object per translation unit, not one
 * per program. This is the distinction that makes `static int counter` in a
 * header produce one counter per includer. */
static int file_counter = 0;

static int bump(void) { return ++file_counter; }

int main(void) {
  CRAFT_ASSERT(helper(5) == 10);
  CRAFT_ASSERT(exported_helper(5) == 6);
  CRAFT_ASSERT(bump() == 1);
  CRAFT_ASSERT(bump() == 2);
  printf("helper(5)=%d exported(5)=%d counter=%d\n", helper(5),
         exported_helper(5), file_counter);

  /* `static` at block scope is a third meaning: the object has static storage
   * duration but the name is scoped to the block. It survives every call. */
  for (int i = 0; i < 3; ++i) {
    static int calls = 0;
    ++calls;
    if (i == 2) {
      CRAFT_ASSERT(calls == 3);
      printf("block-static calls = %d after three iterations\n", calls);
    }
  }

  /* The ODR, stated as a rule: a function defined in a header and included by
   * two units is defined twice, and the guard does not help — the guard makes
   * the definition appear once per unit, which is once too many. */
  printf("headers declare; .c files define; inline is the exception\n");
  return 0;
}
