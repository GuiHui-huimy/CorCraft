/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 16-01  error_handling/01_errno_chain.c
 * Drills: errno is a side channel that any call can clobber, so it must be
 *         read immediately and saved before anything else runs.
 * Build:  cmake --build build --target c_16_errno
 */
#include "craft_assert.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* The wrong pattern: errno is checked after a printf, and printf may set it. */
static int open_wrong(const char *path) {
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    printf("failed to open %s\n", path); /* could clobber errno */
    return errno;                        /* reads whatever is current now */
  }
  fclose(fp);
  return 0;
}

/* The right pattern: capture errno on the next line, before any other call. */
static int open_right(const char *path, int *out_errno) {
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    int saved = errno; /* the first statement after the failure */
    *out_errno = saved;
    return -1;
  }
  fclose(fp);
  return 0;
}

int main(void) {
  const char *missing = "craft_no_such_file_12345.txt";

  int reported = open_wrong(missing);
  printf("wrong pattern reports errno %d (%s)\n", reported,
         strerror(reported));

  int saved = 0;
  int rc = open_right(missing, &saved);
  CRAFT_ASSERT(rc == -1);
  CRAFT_ASSERT(saved == ENOENT);
  printf("right pattern: rc=%d errno=%d (%s)\n", rc, saved, strerror(saved));

  /* errno is not reset by a successful call: it must be set to 0 before the
   * operation if the caller wants to distinguish "failed" from "did nothing
   * but left the previous error in place". */
  errno = 0;
  FILE *fp = fopen("/dev/null", "rb");
  if (fp != NULL) {
    fclose(fp);
    printf("successful open leaves errno at %d\n", errno);
  }

  /* The return-code convention used across this repository: 0 is success,
   * negative is failure, and the detail is reported separately. An error
   * string is for the user; a code is for the caller. */
  printf("convention: 0 success, -1 failure, detail out of band\n");
  return 0;
}
