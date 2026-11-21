/* LAB 10-03  bitops/03_two_bit_code.c
 * Drills: packing bases two bits each, clearing before setting, and reading
 *         back across a word boundary.
 * Build:  cmake --build build --target c_10_twobit
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint64_t *words;
  size_t n_bases;
} packed_seq;

static packed_seq *packed_new(size_t n_bases) {
  packed_seq *s = (packed_seq *)malloc(sizeof *s);
  if (s == NULL) {
    return NULL;
  }
  s->n_bases = n_bases;
  s->words = (uint64_t *)calloc((n_bases + 31u) / 32u, sizeof *s->words);
  if (s->words == NULL) {
    free(s);
    return NULL;
  }
  return s;
}

static void packed_free(packed_seq *s) {
  if (s == NULL) {
    return;
  }
  free(s->words);
  free(s);
}

/* base codes: A=0, C=1, G=2, T=3 */
static uint8_t base_code(char c) {
  switch (c) {
  case 'A':
    return 0u;
  case 'C':
    return 1u;
  case 'G':
    return 2u;
  default:
    return 3u; /* T, and a deliberate default rather than a silent error */
  }
}

static void packed_set(packed_seq *s, size_t i, uint8_t code) {
  size_t w = i / 32u;
  unsigned b = (unsigned)((i % 32u) * 2u);
  /* Clear first, then or: without the clear the low bits of the old value
   * survive and the read back gives a mixture. */
  s->words[w] = (s->words[w] & ~(3ull << b)) | ((uint64_t)(code & 3u) << b);
}

static uint8_t packed_get(const packed_seq *s, size_t i) {
  size_t w = i / 32u;
  unsigned b = (unsigned)((i % 32u) * 2u);
  return (uint8_t)((s->words[w] >> b) & 3ull);
}

int main(void) {
  const char *seq = "ACGTACGTAA";
  size_t n = 10;

  packed_seq *s = packed_new(n);
  CRAFT_ASSERT(s != NULL);

  for (size_t i = 0; i < n; ++i) {
    packed_set(s, i, base_code(seq[i]));
  }

  for (size_t i = 0; i < n; ++i) {
    CRAFT_ASSERT(packed_get(s, i) == base_code(seq[i]));
  }
  printf("%zu bases packed into %zu words\n", n, (n + 31u) / 32u);

  /* Overwrite a base with a different one, then read it back: the clear is
   * what makes this exact. Set A after G, then set A again. */
  packed_set(s, 3, 2u); /* position 3 is T; force G */
  CRAFT_ASSERT(packed_get(s, 3) == 2u);
  packed_set(s, 3, 0u); /* force A */
  CRAFT_ASSERT(packed_get(s, 3) == 0u);
  printf("position 3 after two writes = %u (A)\n", packed_get(s, 3));

  /* A sequence longer than 32 bases crosses a word boundary. */
  size_t long_n = 70;
  packed_seq *l = packed_new(long_n);
  CRAFT_ASSERT(l != NULL);
  for (size_t i = 0; i < long_n; ++i) {
    packed_set(l, i, (uint8_t)(i % 4u));
  }
  for (size_t i = 0; i < long_n; ++i) {
    CRAFT_ASSERT(packed_get(l, i) == (uint8_t)(i % 4u));
  }
  printf("%zu bases across %zu words: round-trip exact\n", long_n,
         (long_n + 31u) / 32u);

  printf("memory: char %zu bytes, packed %zu bytes\n", long_n,
         ((long_n + 31u) / 32u) * sizeof(uint64_t));

  packed_free(s);
  packed_free(l);
  return 0;
}
