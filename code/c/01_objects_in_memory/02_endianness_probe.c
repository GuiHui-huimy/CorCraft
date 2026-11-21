/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 01-02  objects_in_memory/02_endianness_probe.c
 * Drills: byte order of an int, detected two ways, and why it matters for files.
 * Build:  cmake --build build --target c_01_endian
 */
#include <stdint.h>
#include <stdio.h>

/* Method 1: a union aliases the same storage as two types. This is the
 * textbook probe, and it is legal because reading the other member of a union
 * is explicitly permitted. */
static int little_endian_union(void) {
  union {
    uint32_t word;
    uint8_t bytes[4];
  } probe;
  probe.word = 0x01020304u;
  return probe.bytes[0] == 0x04u;
}

/* Method 2: a value that is written and read back through a file is the
 * situation the question actually matters for. */
static uint32_t read_back_native(uint32_t value) {
  uint8_t buf[4];
  for (size_t i = 0; i < 4; ++i) {
    buf[i] = (uint8_t)((value >> (8u * i)) & 0xFFu);
  }
  uint32_t out = 0;
  for (size_t i = 0; i < 4; ++i) {
    out |= (uint32_t)buf[i] << (8u * i);
  }
  return out;
}

int main(void) {
  int little = little_endian_union();
  printf("union probe:      %s endian\n", little ? "little" : "big");

  uint32_t v = 0x01020304u;
  printf("value            0x%08X\n", (unsigned)v);
  printf("native roundtrip 0x%08X  (%s)\n", (unsigned)read_back_native(v),
         read_back_native(v) == v ? "exact" : "changed");

  /* The defect this warns about: a file written as uint32_t and read on a
   * machine with the other byte order gives a plausible-looking wrong number. */
  printf("175132288 as a 4-byte file read as big-endian int\n");
  printf("  little-endian reader sees 0x%08X\n", 0x01020304u);
  printf("  big-endian reader sees    0x%08X\n", 0x04030201u);
  return 0;
}
