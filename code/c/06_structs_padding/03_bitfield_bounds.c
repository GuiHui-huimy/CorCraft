/* LAB 06-03  structs_padding/03_bitfield_bounds.c
 * Drills: bit-field width, the signedness trap, and reading packed fields back.
 * Build:  cmake --build build --target c_06_bits
 *
 * Out-of-range assignment to a bit-field is a conversion the compiler is
 * entitled to warn about, so the wrapping behaviour is demonstrated with an
 * explicit mask instead of by writing 9 into a 3-bit field.
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>

/* A bit-field of width n holds n bits. With a signed type, one of those bits
 * is the sign, so the range is -2^(n-1) .. 2^(n-1)-1, not 0 .. 2^n-1. */
struct Flags {
  unsigned int ready : 1;
  unsigned int mode : 3;  /* 0..7 */
  signed int offset : 4;  /* -8..7 */
  unsigned int : 0;       /* force the next field to a new storage unit */
  unsigned int tag : 8;
};

int main(void) {
  printf("sizeof(struct Flags) = %zu\n", sizeof(struct Flags));

  struct Flags f;
  f.ready = 1;
  f.mode = 5;
  f.offset = -3;
  f.tag = 200;

  CRAFT_ASSERT(f.ready == 1);
  CRAFT_ASSERT(f.mode == 5);
  CRAFT_ASSERT(f.offset == -3);
  CRAFT_ASSERT(f.tag == 200);

  /* What a 3-bit field does to 9: keeps the low three bits, discards the rest,
   * with no warning and no error. */
  unsigned int raw = 9u;
  unsigned int kept = raw & 0x7u;
  CRAFT_ASSERT(kept == 1u);
  printf("9 masked to 3 bits = %u (9 & 0b111)\n", kept);

  /* What a 4-bit signed field does to 8: the top bit becomes the sign, so the
   * stored value is -8 rather than 8. */
  int raw_offset = 8;
  int stored = (raw_offset & 0x8) ? raw_offset - 16 : raw_offset;
  CRAFT_ASSERT(stored == -8);
  printf("8 in a 4-bit signed field reads back %d\n", stored);

  /* A negative value in an unsigned field converts modulo 2^n. The value is
   * computed as unsigned arithmetic rather than assigned, for the same reason. */
  unsigned int negative = (unsigned int)((0u - 1u) & 0x7u);
  CRAFT_ASSERT(negative == 7u);
  printf("-1 in a 3-bit unsigned field reads back %u\n", negative);

  printf("sum of field widths = 1 + 3 + 4 + 8 = 16 bits, but sizeof is %zu\n",
         sizeof(struct Flags));
  return 0;
}
