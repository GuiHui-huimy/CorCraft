/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 08-01  qualifiers/01_alias_violation.c
 * Drills: const in its three positions, strict aliasing, and the restrict
 *         promise.
 * Build:  cmake --build build --target c_08_alias
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>

/* Three different meanings of const. */
static void const_positions(void) {
  int v = 7;
  const int *p_to_const = &v; /* the int cannot be modified through p */
  int *const const_p = &v;    /* the pointer cannot be reseated */
  const int *const both = &v; /* neither */
  CRAFT_ASSERT(*p_to_const == 7);
  CRAFT_ASSERT(*const_p == 7);
  CRAFT_ASSERT(*both == 7);
  printf("three const positions all read %d\n", *both);
}

/* A cast that changes the pointee type breaks the aliasing rule. The compiler
 * is entitled to assume the two names never overlap, so the write through the
 * float pointer is not guaranteed to be visible through the int pointer.
 * The diagnostic is suppressed because the cast is the lesson. */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
static void alias_violation(void) {
  int32_t i = 0x3F800000;
  float *as_float = (float *)&i; /* violates strict aliasing */
  printf("aliased read: %f (works here, not guaranteed)\n", (double)*as_float);
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/* The legal form: a union is the sanctioned way to reinterpret storage. */
static void alias_fixed(void) {
  union {
    int32_t i;
    float f;
  } u;
  u.i = 0x3F800000;
  CRAFT_ASSERT(u.f == 1.0f);
  printf("union read:   %f\n", (double)u.f);
}

/* restrict: the promise that the two pointer ranges do not overlap. It lets
 * the compiler keep a value in a register across a store. */
static void add_restrict(const int *restrict a, const int *restrict b,
                         int *restrict out, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out[i] = a[i] + b[i];
  }
}

int main(void) {
  const_positions();
  alias_violation();
  alias_fixed();

  int a[4] = {1, 2, 3, 4};
  int b[4] = {10, 20, 30, 40};
  int out[4];
  add_restrict(a, b, out, 4);
  CRAFT_ASSERT(out[0] == 11 && out[3] == 44);
  printf("restrict sum: %d %d\n", out[0], out[3]);
  return 0;
}
