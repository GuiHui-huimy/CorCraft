/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 04-01  alignment/nw.c
 * Drills: the Needleman-Wunsch recurrence, the traceback, and the two-row
 *         score-only variant that proves the table can be smaller.
 * Build:  cmake --build build --target bio_04_nw
 *
 * Sequences are short and embedded; the score is the identity score, +1 match,
 * -1 mismatch, -1 per gap character.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>

#define MATCH 1
#define MISS (-1)
#define GAP (-1)

static int sub(char a, char b) { return a == b ? MATCH : MISS; }

static int cell(const int *t, size_t i, size_t j, size_t m) {
  return t[i * (m + 1) + j];
}

/* Full table: the score and the direction per cell, for the traceback. */
static int align_full(const char *a, const char *b, size_t n, size_t m,
                      char *out_a, char *out_b) {
  int *score = (int *)malloc((n + 1) * (m + 1) * sizeof *score);
  unsigned char *dir = (unsigned char *)malloc((n + 1) * (m + 1));
  CRAFT_ASSERT(score != NULL && dir != NULL);

  score[0] = 0;
  for (size_t i = 1; i <= n; ++i) {
    score[i * (m + 1)] = -(int)i; /* i gap characters */
  }
  for (size_t j = 1; j <= m; ++j) {
    score[j] = -(int)j;
  }

  for (size_t i = 1; i <= n; ++i) {
    for (size_t j = 1; j <= m; ++j) {
      int diag = cell(score, i - 1, j - 1, m) + sub(a[i - 1], b[j - 1]);
      int up = cell(score, i - 1, j, m) + GAP;
      int left = cell(score, i, j - 1, m) + GAP;
      int best = diag;
      unsigned char d = 0;
      if (up > best) {
        best = up;
        d = 1;
      }
      if (left > best) {
        best = left;
        d = 2;
      }
      score[i * (m + 1) + j] = best;
      dir[i * (m + 1) + j] = d;
    }
  }

  int total = cell(score, n, m, m);

  /* Traceback from (n, m) to (0, 0), writing backwards. The direction is read
   * only for a full cell, because the boundary cells have no stored direction. */
  size_t i = n, j = m, k = 0;
  while (i > 0 || j > 0) {
    if (i > 0 && j > 0) {
      unsigned char d = dir[i * (m + 1) + j];
      if (d == 0) {
        out_a[k] = a[i - 1];
        out_b[k] = b[j - 1];
        --i;
        --j;
      } else if (d == 1) {
        out_a[k] = a[i - 1];
        out_b[k] = '-';
        --i;
      } else {
        out_a[k] = '-';
        out_b[k] = b[j - 1];
        --j;
      }
    } else if (i > 0) {
      out_a[k] = a[i - 1];
      out_b[k] = '-';
      --i;
    } else {
      out_a[k] = '-';
      out_b[k] = b[j - 1];
      --j;
    }
    ++k;
  }
  out_a[k] = '\0';
  out_b[k] = '\0';

  for (size_t s = 0, t = k - 1; s < t; ++s, --t) {
    char tmp = out_a[s];
    out_a[s] = out_a[t];
    out_a[t] = tmp;
    tmp = out_b[s];
    out_b[s] = out_b[t];
    out_b[t] = tmp;
  }

  free(score);
  free(dir);
  return total;
}

/* Two rows: the same score, O(m) space, no traceback. */
static int align_score_only(const char *a, const char *b, size_t n, size_t m) {
  int *prev = (int *)malloc((m + 1) * sizeof *prev);
  int *cur = (int *)malloc((m + 1) * sizeof *cur);
  CRAFT_ASSERT(prev != NULL && cur != NULL);

  for (size_t j = 0; j <= m; ++j) {
    prev[j] = -(int)j;
  }
  for (size_t i = 1; i <= n; ++i) {
    cur[0] = -(int)i;
    for (size_t j = 1; j <= m; ++j) {
      int diag = prev[j - 1] + sub(a[i - 1], b[j - 1]);
      int up = prev[j] + GAP;
      int left = cur[j - 1] + GAP;
      int best = diag;
      if (up > best) {
        best = up;
      }
      if (left > best) {
        best = left;
      }
      cur[j] = best;
    }
    int *swap = prev;
    prev = cur;
    cur = swap;
  }
  int total = prev[m];
  free(prev);
  free(cur);
  return total;
}

int main(void) {
  const char *a = "ACGTACGT";
  const char *b = "ACGTTCGT"; /* one substitution */

  char al_a[18], al_b[18];
  int full = align_full(a, b, 8, 8, al_a, al_b);
  int two = align_score_only(a, b, 8, 8);

  CRAFT_ASSERT(full == two); /* the two implementations must agree exactly */
  printf("score %d (both tables agree)\n", full);
  printf("a: %s\nb: %s\n", al_a, al_b);

  /* A self-alignment must score n and be trivially aligned. */
  char s_a[18], s_b[18];
  int self = align_full(a, a, 8, 8, s_a, s_b);
  CRAFT_ASSERT(self == 8);
  printf("self score %d, alignment %s / %s\n", self, s_a, s_b);

  /* The single-residue deletion case: b is missing one character. */
  char d_a[18], d_b[18];
  int del = align_full(a, "ACGTCGT", 8, 7, d_a, d_b);
  printf("deletion score %d\na: %s\nb: %s\n", del, d_a, d_b);

  /* The table sizes that decide whether the two-row version is needed. */
  printf("full table %zu bytes for 8x8; for 10000x10000 it is %zu MB\n",
         (size_t)(8 + 1) * (8 + 1) * sizeof(int),
         (size_t)10001 * 10001 * sizeof(int) / (1024u * 1024u));
  return 0;
}
