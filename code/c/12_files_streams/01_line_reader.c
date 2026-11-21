/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 12-01  files_streams/01_line_reader.c
 * Drills: reading a file line by line without a fixed buffer, CRLF handling,
 *         and the truncation that fgets hides.
 * Build:  cmake --build build --target c_12_lines
 */
#include "craft_assert.h"
#include "craft_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The simple version: a fixed buffer. It is correct only if no line is longer
 * than the buffer, and the caller cannot tell when that assumption fails. */
static size_t count_lines_fgets(FILE *fp) {
  char buf[16];
  size_t lines = 0;
  while (fgets(buf, sizeof buf, fp) != NULL) {
    ++lines;
  }
  return lines;
}

/* The version that reports truncation: a line longer than the buffer produces
 * a count that is too large, and the loop detects it. */
static size_t count_lines_checked(FILE *fp) {
  char buf[16];
  size_t lines = 0;
  while (fgets(buf, sizeof buf, fp) != NULL) {
    size_t len = strlen(buf);
    if (len == sizeof buf - 1 && buf[len - 1] != '\n') {
      /* the line continues: consume the rest before counting it once */
      int c;
      while ((c = fgetc(fp)) != EOF && c != '\n') {
      }
    }
    ++lines;
  }
  return lines;
}

/* The whole-file version: one read, then split on the newline, stripping a
 * trailing CR. This is what the FASTA parser does, in one place. */
static size_t count_lines_buffer(const char *buf, size_t len) {
  size_t lines = 0;
  int in_line = 0;
  for (size_t i = 0; i < len; ++i) {
    if (buf[i] == '\n') {
      if (in_line) {
        ++lines;
      }
      in_line = 0;
    } else if (buf[i] != '\r') {
      in_line = 1;
    }
  }
  if (in_line) {
    ++lines; /* a final line with no newline */
  }
  return lines;
}

int main(void) {
  const char *path = "craft_lines_demo.txt";
  const char *content =
      "short\n"
      "a line that is longer than sixteen bytes\n"
      "crlf line\r\n"
      "last without newline";

  CRAFT_ASSERT(craft_write_file(path, content, strlen(content)) == 0);

  size_t len = 0;
  char *buf = craft_read_file(path, &len);
  CRAFT_ASSERT(buf != NULL);
  CRAFT_ASSERT(len == strlen(content));

  CRAFT_ASSERT(count_lines_buffer(buf, len) == 4);
  printf("buffer reader: 4 lines, %zu bytes\n", len);

  FILE *fp = fopen(path, "rb");
  CRAFT_ASSERT(fp != NULL);
  size_t by_fgets = count_lines_fgets(fp);
  fclose(fp);
  printf("fgets with a 16-byte buffer: %zu lines (too many)\n", by_fgets);

  fp = fopen(path, "rb");
  CRAFT_ASSERT(fp != NULL);
  size_t checked = count_lines_checked(fp);
  fclose(fp);
  CRAFT_ASSERT(checked == 4);
  printf("checked fgets: %zu lines (correct)\n", checked);

  free(buf);

  /* The demo file is removed so the working directory stays clean; a lab
   * script would compare the output instead of keeping the file. */
  if (remove(path) != 0) {
    fprintf(stderr, "could not remove %s\n", path);
  }
  return 0;
}
