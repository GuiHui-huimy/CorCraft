/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 07-01  contacts/contact_map.c
 * Drills: the contact-definition decisions, the naive O(n^2) cost, a binned
 *         version that must produce an identical map, and bit storage.
 * Build:  cmake --build build --target bio_07_contacts
 */
#include "craft_assert.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CUTOFF 8.0
#define CUTOFF2 (CUTOFF * CUTOFF)
#define N 64 /* a 64-residue toy structure */

typedef struct {
  double x, y, z;
} vec;

static double dist2(vec a, vec b) {
  double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return dx * dx + dy * dy + dz * dz;
}

/* A contact map as bits: N*N bits = 512 bytes for N = 64, versus 4096 bytes as
 * bool. The indexing is the same word/bit arithmetic as the bit-ops chapter. */
typedef struct {
  uint64_t words[(N * N + 63) / 64];
} bit_map;

static void map_set(bit_map *m, size_t i, size_t j) {
  size_t idx = i * N + j;
  m->words[idx / 64u] |= 1ull << (idx % 64u);
}

static int map_get(const bit_map *m, size_t i, size_t j) {
  size_t idx = i * N + j;
  return (int)((m->words[idx / 64u] >> (idx % 64u)) & 1ull);
}

/* The naive version, with the two decisions that make it right:
 *   j starts at i + 1 (no double counting, no diagonal),
 *   |i - j| <= 2 excluded (backbone neighbours are trivially in contact). */
static size_t contacts_naive(const vec *p, bit_map *m) {
  memset(m, 0, sizeof *m);
  size_t count = 0;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = i + 3; j < N; ++j) {
      if (dist2(p[i], p[j]) < CUTOFF2) {
        map_set(m, i, j);
        map_set(m, j, i);
        ++count;
      }
    }
  }
  return count;
}

/* Binned: cells of side CUTOFF, so a partner must be in the same cell or an
 * adjacent one. The map must come out identical to the naive one. */
static size_t contacts_binned(const vec *p, bit_map *m) {
  memset(m, 0, sizeof *m);

  double minx = p[0].x, miny = p[0].y, minz = p[0].z;
  double maxx = minx, maxy = miny, maxz = minz;
  for (size_t i = 1; i < N; ++i) {
    if (p[i].x < minx) minx = p[i].x;
    if (p[i].y < miny) miny = p[i].y;
    if (p[i].z < minz) minz = p[i].z;
    if (p[i].x > maxx) maxx = p[i].x;
    if (p[i].y > maxy) maxy = p[i].y;
    if (p[i].z > maxz) maxz = p[i].z;
  }

  size_t nx = (size_t)ceil((maxx - minx) / CUTOFF) + 1;
  size_t ny = (size_t)ceil((maxy - miny) / CUTOFF) + 1;
  size_t nz = (size_t)ceil((maxz - minz) / CUTOFF) + 1;

  /* One small structure, so a fixed grid is enough: the index is built by
   * scanning all points once per query, which is the cost the bin removes for
   * large N and does not remove here. The correctness is what this lab tests. */
  size_t count = 0;
  for (size_t i = 0; i < N; ++i) {
    size_t cx = (size_t)((p[i].x - minx) / CUTOFF);
    size_t cy = (size_t)((p[i].y - miny) / CUTOFF);
    size_t cz = (size_t)((p[i].z - minz) / CUTOFF);
    for (size_t j = i + 3; j < N; ++j) {
      size_t jx = (size_t)((p[j].x - minx) / CUTOFF);
      size_t jy = (size_t)((p[j].y - miny) / CUTOFF);
      size_t jz = (size_t)((p[j].z - minz) / CUTOFF);
      size_t dx = cx > jx ? cx - jx : jx - cx;
      size_t dy = cy > jy ? cy - jy : jy - cy;
      size_t dz = cz > jz ? cz - jz : jz - cz;
      if (dx > 1 || dy > 1 || dz > 1) {
        continue; /* cannot be within the cutoff */
      }
      if (dist2(p[i], p[j]) < CUTOFF2) {
        map_set(m, i, j);
        map_set(m, j, i);
        ++count;
      }
    }
  }
  /* The grid dimensions are computed but not materialised: this lab tests that
   * the neighbour test never changes the answer, and the index build is the
   * part a large-N version adds. */
  (void)nx;
  (void)ny;
  (void)nz;
  return count;
}

int main(void) {
  /* A compact blob: points on a 4x4x4 lattice with 3-unit spacing, so many
   * pairs are within 8 units and the map is not sparse. */
  vec points[N];
  size_t k = 0;
  for (size_t x = 0; x < 4; ++x) {
    for (size_t y = 0; y < 4; ++y) {
      for (size_t z = 0; z < 4; ++z) {
        points[k++] = (vec){(double)x * 3.0, (double)y * 3.0, (double)z * 3.0};
      }
    }
  }
  CRAFT_ASSERT(k == N);

  static bit_map naive;
  static bit_map binned;
  size_t a = contacts_naive(points, &naive);
  size_t b = contacts_binned(points, &binned);

  CRAFT_ASSERT(a == b); /* the binning must not change the answer */
  CRAFT_ASSERT(memcmp(naive.words, binned.words, sizeof naive.words) == 0);
  printf("contacts %zu, both maps identical\n", a);

  /* Symmetry, and the absence of the diagonal. */
  CRAFT_ASSERT(map_get(&naive, 0, 1) == 0); /* |i-j| <= 2 excluded */
  CRAFT_ASSERT(map_get(&naive, 5, 5) == 0); /* no self contact */
  size_t i = 0, j = 0;
  CRAFT_ASSERT(map_get(&naive, i, j) == 0);
  /* Find one real contact and check its mirror. */
  int found = 0;
  for (size_t p = 0; p < N && !found; ++p) {
    for (size_t q = p + 3; q < N; ++q) {
      if (map_get(&naive, p, q)) {
        CRAFT_ASSERT(map_get(&naive, q, p));
        found = 1;
        printf("contact pair (%zu, %zu) with mirror set\n", p, q);
        break;
      }
    }
  }
  CRAFT_ASSERT(found);

  /* The storage comparison: bits versus bytes. */
  printf("%d residues: %zu bytes as bits, %zu bytes as bool\n", N,
         sizeof(bit_map), (size_t)N * N);
  printf("cutoff %.1f Angstrom, %zu residue pairs examined\n", CUTOFF,
         (size_t)(N - 2) * (N - 3) / 2);
  return 0;
}
