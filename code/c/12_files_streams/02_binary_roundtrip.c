/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 12-02  files_streams/02_binary_roundtrip.c
 * Drills: writing structs as bytes, the portability problem that creates, and
 *         the field-by-field form that does not.
 * Build:  cmake --build build --target c_12_binary
 */
#include "craft_assert.h"
#include "craft_io.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
  uint32_t id;
  double value;
  char tag; /* 'a' or 'b', kept as one byte in the file */
} packed_record;
#pragma pack(pop)

typedef struct {
  uint32_t id;
  double value;
  char tag;
} native_record;

/* Writes the fields one at a time, with explicit widths and an explicit byte
 * order. This is what survives a different compiler or architecture. */
static size_t encode(const native_record *r, uint8_t *out) {
  size_t n = 0;
  for (unsigned i = 0; i < 4u; ++i) {
    out[n++] = (uint8_t)((r->id >> (8u * i)) & 0xFFu); /* explicit little-endian */
  }
  uint64_t bits;
  memcpy(&bits, &r->value, sizeof bits); /* type pun through memcpy, not a cast */
  for (unsigned i = 0; i < 8u; ++i) {
    out[n++] = (uint8_t)((bits >> (8u * i)) & 0xFFu);
  }
  out[n++] = (uint8_t)r->tag;
  return n;
}

static void decode(const uint8_t *in, native_record *r) {
  size_t n = 0;
  r->id = 0;
  for (unsigned i = 0; i < 4u; ++i) {
    r->id |= (uint32_t)in[n++] << (8u * i);
  }
  uint64_t bits = 0;
  for (unsigned i = 0; i < 8u; ++i) {
    bits |= (uint64_t)in[n++] << (8u * i);
  }
  memcpy(&r->value, &bits, sizeof bits);
  r->tag = (char)in[n++];
}

int main(void) {
  printf("sizeof(native_record) = %zu, packed = %zu (1 + 7 pad + 8 + 1 + 7 pad)\n",
         sizeof(native_record), sizeof(packed_record));

  native_record a = {42u, 3.5, 'a'};
  uint8_t bytes[13];
  size_t used = encode(&a, bytes);
  CRAFT_ASSERT(used == 13);

  native_record b;
  decode(bytes, &b);
  CRAFT_ASSERT(b.id == 42u);
  CRAFT_ASSERT(b.value == 3.5);
  CRAFT_ASSERT(b.tag == 'a');
  printf("roundtrip: id=%u value=%.2f tag=%c\n", b.id, b.value, b.tag);

  /* The file is written and read back, so the test covers the disk as well as
   * the arithmetic. */
  const char *path = "craft_record.bin";
  CRAFT_ASSERT(craft_write_file(path, bytes, used) == 0);

  size_t len = 0;
  char *raw = craft_read_file(path, &len);
  CRAFT_ASSERT(raw != NULL);
  CRAFT_ASSERT(len == used);

  native_record c;
  decode((const uint8_t *)raw, &c);
  CRAFT_ASSERT(c.id == a.id);
  CRAFT_ASSERT(c.value == a.value);
  printf("from file: id=%u value=%.2f\n", c.id, c.value);

  /* The portability note: writing the struct itself would produce 24 bytes on
   * this machine and a different layout elsewhere. The 13 bytes above are the
   * same everywhere. */
  printf("native dump would be %zu bytes and unportable\n",
         sizeof(native_record));

  free(raw);
  (void)remove(path);
  return 0;
}
