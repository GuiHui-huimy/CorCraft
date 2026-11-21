/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 05-01  strings/01_strlen_reimpl.c
 * Drills: what strlen actually does, the off-by-one at the terminator, and the
 *         cost of rescanning a prefix inside a loop.
 * Build:  cmake --build build --target c_05_strlen
 */
#include "craft_assert.h"
#include "craft_bench.h"

#include <stdio.h>

static size_t length_of(const char *s) {
  const char *p = s;
  while (*p != '\0') {
    ++p;
  }
  return (size_t)(p - s);
}

/* The defective version: the loop condition is written == instead of !=.
 * With a non-empty string it stops immediately and reports 0. */
static size_t length_buggy(const char *s) {
  const char *p = s;
  while (*p == '\0') {
    ++p;
  }
  return (size_t)(p - s);
}

/* The quadratic pattern: strlen of the whole string on every iteration.
 * For a string of length n the loop runs n times and each call walks n
 * characters, so the total is n * n — not n * (n + 1) / 2, which is the
 * triangular number and a different quantity. */
static size_t total_lengths_quadratic(const char *s) {
  size_t total = 0;
  for (size_t i = 0; s[i] != '\0'; ++i) {
    total += length_of(s); /* rescans from the start each time */
  }
  return total;
}

static size_t total_lengths_linear(const char *s) {
  size_t n = length_of(s);
  return n * n;
}

int main(void) {
  const char *s = "abcdefghij";

  CRAFT_ASSERT(length_of(s) == 10);
  CRAFT_ASSERT(length_of("") == 0); /* empty string: the loop never runs */
  printf("length_of(\"%s\") = %zu\n", s, length_of(s));

  CRAFT_ASSERT(length_buggy(s) == 0); /* the defect, asserted so it cannot hide */
  printf("length_buggy(\"%s\") = %zu  (wrong)\n", s, length_buggy(s));

  CRAFT_ASSERT(total_lengths_quadratic(s) == total_lengths_linear(s));

  double t0 = craft_cpu_seconds();
  size_t q = total_lengths_quadratic(s);
  double t1 = craft_cpu_seconds();
  size_t l = total_lengths_linear(s);
  double t2 = craft_cpu_seconds();

  printf("quadratic %zu %.8f s\n", q, t1 - t0);
  printf("linear    %zu %.8f s\n", l, t2 - t1);

  printf("sizeof(s) is %zu: the pointer, not the string\n", sizeof(s));
  return 0;
}
