#ifndef CRAFT_IO_H
#define CRAFT_IO_H

#include <stdio.h>
#include <stdlib.h>

/*
 * craft_read_file() reads the whole file into a NUL-terminated buffer and
 * writes its byte length to *out_len when out_len is not NULL.
 *
 * Returns NULL on any failure: open, allocation, or read. The caller owns the
 * buffer and must free() it. Binary files are fine; the trailing NUL is an
 * extra byte and is not counted in *out_len.
 */
static inline char *craft_read_file(const char *path, size_t *out_len) {
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    return NULL;
  }

  size_t cap = 8192;
  size_t len = 0;
  char *buf = (char *)malloc(cap);
  if (buf == NULL) {
    fclose(fp);
    return NULL;
  }

  for (;;) {
    if (len + 1 >= cap) {
      size_t next = cap * 2;
      char *grown = (char *)realloc(buf, next);
      if (grown == NULL) {
        free(buf);
        fclose(fp);
        return NULL;
      }
      buf = grown;
      cap = next;
    }
    size_t got = fread(buf + len, 1, cap - len - 1, fp);
    len += got;
    if (got == 0) {
      break;
    }
  }

  fclose(fp);
  buf[len] = '\0';
  if (out_len != NULL) {
    *out_len = len;
  }
  return buf;
}

/*
 * craft_write_file() is the counterpart used by the file labs. It returns 0 on
 * success and -1 on failure, matching the return-code convention taught in
 * docs/c/error-handling.md.
 */
static inline int craft_write_file(const char *path, const void *data,
                                   size_t len) {
  FILE *fp = fopen(path, "wb");
  if (fp == NULL) {
    return -1;
  }
  size_t wrote = fwrite(data, 1, len, fp);
  int failed = (wrote != len) || (fclose(fp) != 0);
  return failed ? -1 : 0;
}

#endif /* CRAFT_IO_H */
