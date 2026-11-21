/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 10-02  bitops/02_popcount.c
 * Drills: three population-count implementations, and why the table version
 *         wins at the sizes this book uses.
 * Build:  cmake --build build --target c_10_popcount
 */
#include "craft_assert.h"
#include "craft_bench.h"

#include <stdint.h>
#include <stdio.h>

/* The naive version: one branch per bit, 32 iterations per word. */
static int popcount_naive(uint32_t x) {
  int count = 0;
  for (unsigned i = 0; i < 32u; ++i) {
    if ((x >> i) & 1u) {
      ++count;
    }
  }
  return count;
}

/* Kernighan's version: one iteration per set bit, because x & (x-1) clears
 * the lowest set bit. Sparse values are much faster. */
static int popcount_kernighan(uint32_t x) {
  int count = 0;
  while (x != 0u) {
    x &= x - 1u;
    ++count;
  }
  return count;
}

/* The table version: 256 bytes, one lookup per byte, four lookups per word. */
static uint8_t table[256];

static void table_build(void) {
  for (uint32_t x = 0; x < 256u; ++x) {
    table[x] = (uint8_t)popcount_naive(x);
  }
}

static int popcount_table(uint32_t x) {
  return (int)(table[x & 0xFFu] + table[(x >> 8) & 0xFFu] +
               table[(x >> 16) & 0xFFu] + table[(x >> 24) & 0xFFu]);
}

int main(void) {
  table_build();

  /* The table is checked against the naive version before it is timed. */
  for (uint32_t x = 0; x < 256u; ++x) {
    CRAFT_ASSERT(table[x] == (uint8_t)popcount_naive(x));
  }

  CRAFT_ASSERT(popcount_naive(0u) == 0);
  CRAFT_ASSERT(popcount_naive(0xFFFFFFFFu) == 32);
  CRAFT_ASSERT(popcount_kernighan(0xF0F0F0F0u) == 16);
  CRAFT_ASSERT(popcount_table(0x80000001u) == 2);
  printf("0xF0F0F0F0 has %d set bits\n", popcount_kernighan(0xF0F0F0F0u));

  /* The timing uses a dense word, where Kernighan's advantage disappears. */
  volatile int sink = 0;
  const uint32_t dense = 0xFFFFFFFFu;
  const int repeats = 1000000;

  double t0 = craft_cpu_seconds();
  for (int i = 0; i < repeats; ++i) {
    sink += popcount_naive(dense);
  }
  double t1 = craft_cpu_seconds();

  for (int i = 0; i < repeats; ++i) {
    sink += popcount_kernighan(dense);
  }
  double t2 = craft_cpu_seconds();

  for (int i = 0; i < repeats; ++i) {
    sink += popcount_table(dense);
  }
  double t3 = craft_cpu_seconds();

  printf("naive      %.4f s\n", t1 - t0);
  printf("kernighan  %.4f s\n", t2 - t1);
  printf("table      %.4f s\n", t3 - t2);
  printf("sink       %d\n", sink);
  return 0;
}
