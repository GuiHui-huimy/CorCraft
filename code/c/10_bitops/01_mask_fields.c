/* LAB 10-01  bitops/01_mask_fields.c
 * Drills: put / get / clear / toggle on a named bit range, with no branch.
 * Build:  cmake --build build --target c_10_mask
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>

/* Extract a field of width w starting at bit offset, from a 32-bit word. */
static uint32_t field_get(uint32_t word, unsigned offset, unsigned width) {
  uint32_t mask = (width == 32u) ? 0xFFFFFFFFu : ((1u << width) - 1u);
  return (word >> offset) & mask;
}

/* Write the field, leaving every other bit untouched. The two steps are
 * clearing and then or-ing, and forgetting the clear is the classic defect:
 * the old bits survive and the new value is a union of both. */
static uint32_t field_put(uint32_t word, unsigned offset, unsigned width,
                          uint32_t value) {
  uint32_t mask = (width == 32u) ? 0xFFFFFFFFu : ((1u << width) - 1u);
  uint32_t shifted = mask << offset;
  return (word & ~shifted) | ((value & mask) << offset);
}

static int bit_get(uint32_t word, unsigned bit) {
  return (int)((word >> bit) & 1u);
}

static uint32_t bit_set(uint32_t word, unsigned bit) {
  return word | (1u << bit);
}

static uint32_t bit_clear(uint32_t word, unsigned bit) {
  return word & ~(1u << bit);
}

static uint32_t bit_toggle(uint32_t word, unsigned bit) {
  return word ^ (1u << bit);
}

int main(void) {
  uint32_t word = 0u;

  word = field_put(word, 0, 3, 5u);   /* low three bits = 5 */
  word = field_put(word, 8, 4, 12u);  /* bits 8..11 = 12 */
  printf("word = 0x%08X\n", word);

  CRAFT_ASSERT(field_get(word, 0, 3) == 5u);
  CRAFT_ASSERT(field_get(word, 8, 4) == 12u);

  /* Overwrite the same field: the clear is what makes the new value exact. */
  word = field_put(word, 0, 3, 2u);
  CRAFT_ASSERT(field_get(word, 0, 3) == 2u);
  CRAFT_ASSERT(field_get(word, 8, 4) == 12u); /* the neighbour is untouched */

  /* The defect without the clear: an OR leaves both values' bits set. */
  uint32_t stale = word | (3u << 0); /* 2 | 3 = 3, not the 3 we wanted either */
  CRAFT_ASSERT(stale == (word | 3u));
  printf("without clear:   0x%08X  with clear: 0x%08X\n", stale, word);

  word = bit_set(word, 31);
  CRAFT_ASSERT(bit_get(word, 31) == 1);
  word = bit_toggle(word, 31);
  CRAFT_ASSERT(bit_get(word, 31) == 0);
  word = bit_clear(word, 8);
  CRAFT_ASSERT((word & (1u << 8)) == 0u);

  printf("after bit ops:   0x%08X\n", word);
  printf("bit 31 of 0x80000000 is %d\n", bit_get(0x80000000u, 31));

  /* A width of 32 needs the special case: 1u << 32 is undefined. */
  printf("full-word field: 0x%08X\n", field_get(0xDEADBEEFu, 0, 32));
  return 0;
}
