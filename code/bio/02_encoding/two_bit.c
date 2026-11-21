/* LAB 02-02  encoding/two_bit.c
 * Drills: two bits per base, the read-modify-write that must clear first, and
 *         the memory arithmetic that decides when packing is worth it.
 * Build:  cmake --build build --target bio_02_twobit
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint64_t *words;
  size_t n_bases;
} packed;

static packed *packed_new(size_t n_bases) {
  packed *p = (packed *)malloc(sizeof *p);
  if (p == NULL) {
    return NULL;
  }
  p->n_bases = n_bases;
  size_t words = (n_bases + 31u) / 32u; /* 32 bases per 64-bit word */
  p->words = (uint64_t *)calloc(words, sizeof *p->words);
  if (p->words == NULL) {
    free(p);
    return NULL;
  }
  return p;
}

static void packed_free(packed *p) {
  if (p == NULL) {
    return;
  }
  free(p->words);
  free(p);
}

static void put(packed *p, size_t i, uint64_t code) {
  size_t w = i / 32u;
  unsigned shift = (unsigned)((i % 32u) * 2u);
  uint64_t mask = 3ull << shift;
  p->words[w] = (p->words[w] & ~mask) | ((code & 3ull) << shift);
}

static uint64_t get(const packed *p, size_t i) {
  size_t w = i / 32u;
  unsigned shift = (unsigned)((i % 32u) * 2u);
  return (p->words[w] >> shift) & 3ull;
}

int main(void) {
  /* A 5000-base sequence: 5000 bytes as chars, 1256 bytes packed. */
  const size_t n = 5000;
  packed *p = packed_new(n);
  CRAFT_ASSERT(p != NULL);

  for (size_t i = 0; i < n; ++i) {
    put(p, i, (uint64_t)(i % 4u));
  }
  for (size_t i = 0; i < n; ++i) {
    CRAFT_ASSERT(get(p, i) == (uint64_t)(i % 4u));
  }

  size_t packed_bytes = ((n + 31u) / 32u) * sizeof(uint64_t);
  printf("chars %zu bytes, packed %zu bytes, ratio %.2f\n", n, packed_bytes,
         (double)n / (double)packed_bytes);

  /* The overwrite that proves the clear is present: write 3 then 0 to the same
   * slot and read 0. Without the clear, the 3's low bit would survive. */
  put(p, 0, 3ull);
  CRAFT_ASSERT(get(p, 0) == 3ull);
  put(p, 0, 0ull);
  CRAFT_ASSERT(get(p, 0) == 0ull);
  printf("overwrite 3 -> 0 at position 0: exact\n");

  /* The word boundary: position 31 is the last base of word 0, position 32 the
   * first of word 1. A packing that used 30 bases per word would be a different
   * format and a different file. */
  put(p, 31, 1ull);
  put(p, 32, 2ull);
  CRAFT_ASSERT(get(p, 31) == 1ull);
  CRAFT_ASSERT(get(p, 32) == 2ull);
  printf("word boundary at 32: positions 31 and 32 independent\n");

  /* The break-even argument: for a single 76-residue protein the shifts cost
   * more than the 57 bytes saved, so packing is a storage decision rather than
   * a compute decision. */
  printf("76-residue protein: 76 chars vs %zu packed bytes\n",
         ((size_t)76 + 31u) / 32u * sizeof(uint64_t));

  packed_free(p);
  return 0;
}
