/* LAB 02-01  encoding/gc_kmer.c
 * Drills: composition from a 26-slot table, the GC fraction's two decisions,
 *         and a k-mer count in base-4 arithmetic.
 * Build:  cmake --build build --target bio_02_gc
 */
#include "craft_assert.h"

#include <stdio.h>

#define KMER3 64 /* 4^3 */

/* The index computation is where an off-by-one becomes a buffer overflow: a
 * lowercase character minus 'A' is 32, eight past the end of counts[26]. */
static int letter_index(char c) {
  if (c >= 'A' && c <= 'Z') {
    return (int)(c - 'A');
  }
  if (c >= 'a' && c <= 'z') {
    return (int)(c - 'a');
  }
  return -1;
}

static int is_residue_letter(char c) {
  static const char *letters = "ACDEFGHIKLMNPQRSTVWY";
  for (const char *p = letters; *p != '\0'; ++p) {
    if (*p == c) {
      return 1;
    }
  }
  return 0;
}

static int base_code(char c) {
  switch (c) {
  case 'A':
    return 0;
  case 'C':
    return 1;
  case 'G':
    return 2;
  default:
    return 3; /* T, and everything else, by the lab's convention */
  }
}

int main(void) {
  const char *seq = "MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQR"
                    "LIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG";
  size_t n = 0;
  while (seq[n] != '\0') {
    ++n;
  }
  CRAFT_ASSERT(n == 76); /* ubiquitin */

  size_t counts[26] = {0};
  for (size_t i = 0; i < n; ++i) {
    int idx = letter_index(seq[i]);
    CRAFT_ASSERT(idx >= 0 && idx < 26);
    ++counts[idx];
  }

  size_t gc = counts['G' - 'A'] + counts['C' - 'A'];
  size_t residues = 0;
  for (int i = 0; i < 26; ++i) {
    char letter = (char)('A' + i);
    if (is_residue_letter(letter)) {
      residues += counts[i];
    }
  }
  CRAFT_ASSERT(residues == n); /* no non-residue characters in this input */
  double fraction = (double)gc / (double)residues;
  printf("GC %zu of %zu = %.4f\n", gc, residues, fraction);

  /* The defect the denominator choice prevents: dividing by the length
   * including a stop codon would report a smaller fraction. */
  CRAFT_ASSERT(n == 76);

  /* k-mer counting in base 4: index = 16*a + 4*b + c, no hash, no string key. */
  const char *dna = "ACGTTGCAACGTTGCA";
  size_t kmer[KMER3] = {0};
  size_t dna_n = 16;
  for (size_t i = 0; i + 3 <= dna_n; ++i) {
    int code = 16 * base_code(dna[i]) + 4 * base_code(dna[i + 1]) +
               base_code(dna[i + 2]);
    CRAFT_ASSERT(code >= 0 && code < KMER3);
    ++kmer[code];
  }

  size_t total = 0;
  for (int i = 0; i < KMER3; ++i) {
    total += kmer[i];
  }
  CRAFT_ASSERT(total == dna_n - 2); /* n - k + 1 windows */
  printf("3-mers counted %zu, table bytes %zu\n", total,
         sizeof kmer);

  printf("ACG = code %d, GCA = code %d\n",
         16 * base_code('A') + 4 * base_code('C') + base_code('G'),
         16 * base_code('G') + 4 * base_code('C') + base_code('A'));
  return 0;
}
