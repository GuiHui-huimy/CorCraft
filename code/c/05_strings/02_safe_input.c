/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 05-02  strings/02_safe_input.c
 * Drills: bounded input, the truncation boundary, and why gets() cannot be
 *         used. The lab supplies the lines instead of reading a terminal.
 * Build:  cmake --build build --target c_05_input
 */
#include "craft_assert.h"

#include <stdio.h>
#include <string.h>

/* Copies at most cap-1 characters and reports whether the line fit.
 * Returns 0 when the whole line was stored, -1 when it was truncated. */
static int read_field(const char *line, char *out, size_t cap) {
  CRAFT_ASSERT(cap > 0);
  size_t n = strlen(line);
  if (n >= cap) {
    memcpy(out, line, cap - 1);
    out[cap - 1] = '\0';
    return -1;
  }
  memcpy(out, line, n + 1);
  return 0;
}

int main(void) {
  char out[8];

  CRAFT_ASSERT(read_field("abc", out, sizeof out) == 0);
  CRAFT_ASSERT(strcmp(out, "abc") == 0);
  printf("short line  -> \"%s\"\n", out);

  /* Exactly cap-1 characters: the largest line that fits. */
  CRAFT_ASSERT(read_field("abcdefg", out, sizeof out) == 0);
  CRAFT_ASSERT(strlen(out) == 7);
  printf("full line   -> \"%s\" (7 chars, cap 8)\n", out);

  /* One character more: truncated, and the caller is told. */
  CRAFT_ASSERT(read_field("abcdefgh", out, sizeof out) == -1);
  CRAFT_ASSERT(strlen(out) == 7);
  printf("too long    -> \"%s\" (truncated, return -1)\n", out);

  /* An empty line is a legal field, not an error. */
  CRAFT_ASSERT(read_field("", out, sizeof out) == 0);
  CRAFT_ASSERT(out[0] == '\0');
  printf("empty line  -> \"%s\"\n", out);

  /* The three rules this lab encodes:
   *   1. every copy has a capacity,
   *   2. truncation is reported, not silent,
   *   3. an empty input is distinguished from a failure. */
  return 0;
}
