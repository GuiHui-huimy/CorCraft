/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 11-01  callbacks/01_qsort_words.c
 * Drills: a comparison function as a function pointer, and the two defects
 *         inside a comparator written in a hurry.
 * Build:  cmake --build build --target c_11_qsort
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int by_length(const void *pa, const void *pb) {
  const char *a = *(const char *const *)pa;
  const char *b = *(const char *const *)pb;
  size_t la = strlen(a), lb = strlen(b);
  if (la < lb) {
    return -1;
  }
  if (la > lb) {
    return 1;
  }
  return strcmp(a, b); /* tie broken by content, so the order is total */
}

/* The defective comparator: it returns a difference of ints, which overflows
 * and, for long strings, reports the wrong sign. */
static int by_length_buggy(const void *pa, const void *pb) {
  int la = (int)strlen(*(const char *const *)pa);
  int lb = (int)strlen(*(const char *const *)pb);
  return la - lb;
}

int main(void) {
  const char *words[] = {"delta", "a", "echoing", "bc", "charlie"};
  size_t n = sizeof words / sizeof words[0];

  qsort(words, n, sizeof words[0], by_length);
  printf("sorted: ");
  for (size_t i = 0; i < n; ++i) {
    printf("%s ", words[i]);
  }
  printf("\n");

  CRAFT_ASSERT(strcmp(words[0], "a") == 0);
  CRAFT_ASSERT(strcmp(words[n - 1], "echoing") == 0);

  /* The comparator contract: negative, zero, positive must be consistent with
   * a < b, a == b, a > b. A difference-based comparator satisfies it only
   * while the difference fits in an int. */
  CRAFT_ASSERT(by_length(&words[0], &words[1]) < 0);
  CRAFT_ASSERT(by_length(&words[1], &words[1]) == 0);
  printf("buggy comparator on the same pair: %d\n",
         by_length_buggy(&words[0], &words[1]));

  /* qsort is not stable: equal elements may end up in any order. The
   * stable_key_sort.c lab is the fix for the case where that matters. */
  printf("qsort is unstable; equal lengths need a tie-break or a stable sort\n");
  return 0;
}
