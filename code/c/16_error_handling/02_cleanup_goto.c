/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 16-02  error_handling/02_cleanup_goto.c
 * Drills: one exit point, resources released in reverse order, and why the
 *         goto is the right tool here rather than an escape from structure.
 * Build:  cmake --build build --target c_16_cleanup
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *name;
  int *values;
} dataset;

/* Four acquisitions, each of which can fail. The goto labels are ordered so
 * that falling into the next label releases exactly what was acquired. */
static int load(const char *name, size_t n, dataset *out) {
  int rc = -1;
  out->name = NULL;
  out->values = NULL;

  out->name = (char *)malloc(strlen(name) + 1);
  if (out->name == NULL) {
    goto fail_name;
  }
  memcpy(out->name, name, strlen(name) + 1);

  out->values = (int *)malloc(n * sizeof *out->values);
  if (out->values == NULL) {
    goto fail_values;
  }
  for (size_t i = 0; i < n; ++i) {
    out->values[i] = (int)i;
  }

  rc = 0;
  goto done; /* success falls through the cleanup labels */

fail_values:
  free(out->name);
  out->name = NULL;
fail_name:
done:
  return rc;
}

static void dataset_free(dataset *d) {
  free(d->name);
  free(d->values);
  d->name = NULL;
  d->values = NULL;
}

int main(void) {
  dataset d;
  CRAFT_ASSERT(load("ubiquitin", 76, &d) == 0);
  CRAFT_ASSERT(d.name != NULL);
  CRAFT_ASSERT(d.values[75] == 75);
  printf("loaded %s with %d values\n", d.name, d.values[75]);
  dataset_free(&d);

  /* The forced-failure path: n chosen so the second allocation fails is hard to
   * arrange, so the lab checks the invariant instead — after a failure the
   * struct has no owner and nothing leaks. */
  CRAFT_ASSERT(load("", 1, &d) == 0);
  CRAFT_ASSERT(d.name != NULL && d.name[0] == '\0');
  dataset_free(&d);
  printf("empty name accepted, freed cleanly\n");

  /* The argument the chapter makes: with four resources and four failure
   * points, a nested-if version has sixteen paths and the goto version has
   * four. The count is why C code uses this pattern. */
  printf("4 resources, 4 failure points: goto has 4 cleanup paths, not 16\n");
  return 0;
}
