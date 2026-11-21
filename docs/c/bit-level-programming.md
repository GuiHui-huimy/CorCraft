# Bit-Level Programming

Bit work is where the type system and the hardware disagree most often, so I start with masks and fields and keep everything unsigned until the boundary. `popcount` comes in three versions because the difference between them is a measurement, not a preference, and the two-bit packing section is the same idea applied to a biological sequence. Bit order is stated as an assumption rather than left implicit, and the shift rules are the ones I have seen fail in code review.

Bit operations are the one part of C where the language's model of an integer as a bag of bits becomes explicit. They are also the part that the biology section leans on hardest: a nucleotide alphabet has four letters, which is exactly two bits, and packing a sequence into a word is the difference between 4 GB and 1 GB of memory for a genome-scale library.

## 1. Masks and fields

Extracting a field from a packed word is two operations: shift down to the low end, then mask off everything above the field.

```c
/* LAB 10-01  bitops/01_mask_fields.c */
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t word = 0xABCD1234u;
    unsigned low  = word & 0xFFu;              /* 0x34 */
    unsigned next = (word >> 8) & 0xFFu;       /* 0x12 */
    unsigned high = (word >> 24) & 0xFFu;      /* 0xAB */
    printf("%02X %02X %02X\n", low, next, high);
    return 0;
}
```

Writing a field is the mirror image: clear the field with an inverted mask, then OR in the shifted value.

```c
word = (word & ~(0xFFu << 8)) | ((value & 0xFFu) << 8);
```

The `value & 0xFFu` before the shift is not decoration. Without it, a `value` larger than 255 carries bits into the neighbouring field, and the corruption is silent because the OR succeeds.

## 2. Unsigned is the type for bit work

Shifting and masking on signed integers runs into the rules from the previous chapter: left-shifting into the sign bit is UB, and right-shifting a negative value is implementation-defined. The fix is to do all bit work in `uint32_t` or `uint64_t` and cast only at the boundary.

```c
uint32_t read_be_u32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}
```

That function is the portable way to read a big-endian word. Note the casts before the shifts: `p[0]` promotes to `int`, and `(p[0]) << 24` with a byte above 127 shifts a value into the sign bit. Casting to `uint32_t` first makes the operation defined.

## 3. `popcount`, three ways

Counting set bits is the standard micro-benchmark for bit tricks, and the three implementations differ by a factor of about thirty.

```c
/* LAB 10-02  bitops/02_popcount.c */
static unsigned count_naive(uint32_t v) {
    unsigned n = 0;
    for (int i = 0; i < 32; ++i) { n += (v >> i) & 1u; }
    return n;
}

static unsigned count_kernighan(uint32_t v) {
    unsigned n = 0;
    while (v != 0u) { v &= v - 1u; ++n; }     /* clears the lowest set bit */
    return n;
}
```

Kernighan's loop runs once per set bit rather than once per bit position, so a sparse word is fast and a dense one is not. For a word with all 32 bits set it performs 32 iterations of a cheaper body; for a word with one bit set, one iteration.

Modern compilers recognise both idioms and may emit a single `popcnt` instruction when the target supports it (`-mpopcnt` on x86-64). The measurement in the lab compares naive, Kernighan, and `__builtin_popcount`, and the third is usually fastest because it is the instruction.

## 4. Packing a nucleotide sequence

The alphabet is `A`, `C`, `G`, `T`; the encoding is two bits per base; sixteen bases fit in one `uint32_t`.

```c
/* LAB 10-03  bitops/03_two_bit_code.c */
static uint32_t pack16(const char *seq) {
    uint32_t word = 0;
    for (int i = 0; i < 16; ++i) {
        uint32_t code = base_code(seq[i]);          /* 0..3 */
        word |= code << (2 * (15 - i));             /* first base in the high bits */
    }
    return word;
}
```

Two decisions are embedded in that loop and both matter later. Placing the first base in the high bits makes the packed word print in sequence order when read left to right, which is what makes debugging bearable. Encoding `A` as `0` rather than `1` makes a word of all `A` equal to zero, which makes a zeroed buffer a valid sequence rather than a sentinel.

The unpacking function is the inverse, and the pair is what the encoding chapter of the biology section benchmarks: 3 billion bases in one byte-per-base representation is 3 GB, and in two-bit packing it is 750 MB — which is the difference between a library that fits in memory and one that does not.

## 5. Bit order: the assumption to state

The loop above assumes that base `i` of the sequence is stored in bits `2(15−i)`, i.e. the first base is the most significant pair. That is a choice, and every file format that stores packed sequence makes the same choice or the opposite one. Nothing in C determines it; the code declares it, and the parser must agree.

The failure mode when the two disagree is not a crash. It is a decoded sequence that is reversed or shuffled, which looks like a biology problem and is a documentation problem. This is why `03_two_bit_code.c` carries a round-trip assertion: pack, unpack, compare to the original string.

## 6. Shifts that are defined

| Expression | Defined? | Note |
| --- | --- | --- |
| `1u << 31` | Yes | The top bit of an unsigned word |
| `1 << 31` | No | Signed, so it overflows |
| `x >> 3` for negative `x` | Implementation-defined | GCC and MSVC both do arithmetic shift |
| `x << 32` on `uint32_t` | No | Shift count must be < width |
| `(uint32_t)x << 31` | Yes | Cast first, then shift |

## 7. Field note: the mask that forgot the range

An early version of the packing routine wrote the base code without masking the input:

```c
word |= (code << shift);       /* code came from a table lookup */
```

The table was indexed by the character, and an unexpected character — a `N` for an unknown base — landed outside the table and produced a code of 4. Shifting 4 into a two-bit field carried into the neighbouring base, so a single ambiguous base silently corrupted two residues of the packed sequence.

The repair was two lines: map unknown characters to a separate `N` bitmask outside the packed word, and mask every code with `& 3u` before shifting. The general lesson is that a packed word has no bounds checking of its own; the width of the field exists only in the mask, and a missing mask is a write into the next field.

## Continue

- [Undefined Behavior](undefined-behavior.md) is the chapter that explains which of these shifts abort.
- [Sequence Encoding](../bio/sequence-encoding.md) applies the packing to real FASTA records.
- [Contacts](../bio/contacts.md) uses the same word-level reasoning for a different purpose: a bit per residue pair.
