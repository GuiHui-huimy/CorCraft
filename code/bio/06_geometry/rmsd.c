/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 06-02  geometry/rmsd.c
 * Drills: the radius of gyration, the difference between a raw RMSD and one
 *         after superposition, and the centroid computed twice.
 * Build:  cmake --build build --target bio_06_rmsd
 */
#include "craft_assert.h"

#include <math.h>
#include <stdio.h>

typedef struct {
  double x, y, z;
} vec;

static vec sub(vec a, vec b) { return (vec){a.x - b.x, a.y - b.y, a.z - b.z}; }
static double dist2(vec a, vec b) {
  vec d = sub(a, b);
  return d.x * d.x + d.y * d.y + d.z * d.z;
}

/* Radius of gyration: the root mean square distance from the centroid. Two
 * passes, and the centroid is computed before the sum. */
static double radius_of_gyration(const vec *p, size_t n) {
  vec cm = {0.0, 0.0, 0.0};
  for (size_t i = 0; i < n; ++i) {
    cm.x += p[i].x;
    cm.y += p[i].y;
    cm.z += p[i].z;
  }
  cm.x /= (double)n;
  cm.y /= (double)n;
  cm.z /= (double)n;

  double sum = 0.0;
  for (size_t i = 0; i < n; ++i) {
    sum += dist2(p[i], cm);
  }
  return sqrt(sum / (double)n);
}

static double rmsd_raw(const vec *a, const vec *b, size_t n) {
  double sum = 0.0;
  for (size_t i = 0; i < n; ++i) {
    sum += dist2(a[i], b[i]);
  }
  return sqrt(sum / (double)n);
}

/* The optimal rigid translation: move both sets to their centroids. This is
 * the part of superposition that needs no linear algebra, and it is enough to
 * show that a raw RMSD is not a structural difference. */
static double rmsd_after_translation(const vec *a, const vec *b, size_t n) {
  vec ca = {0.0, 0.0, 0.0};
  vec cb = {0.0, 0.0, 0.0};
  for (size_t i = 0; i < n; ++i) {
    ca.x += a[i].x;
    ca.y += a[i].y;
    ca.z += a[i].z;
    cb.x += b[i].x;
    cb.y += b[i].y;
    cb.z += b[i].z;
  }
  ca.x /= (double)n;
  ca.y /= (double)n;
  ca.z /= (double)n;
  cb.x /= (double)n;
  cb.y /= (double)n;
  cb.z /= (double)n;

  double sum = 0.0;
  for (size_t i = 0; i < n; ++i) {
    vec pa = {a[i].x - ca.x, a[i].y - ca.y, a[i].z - ca.z};
    vec pb = {b[i].x - cb.x, b[i].y - cb.y, b[i].z - cb.z};
    sum += dist2(pa, pb);
  }
  return sqrt(sum / (double)n);
}

int main(void) {
  /* A straight chain of five points, 1 unit apart. */
  vec chain[5] = {{0.0, 0.0, 0.0},
                  {1.0, 0.0, 0.0},
                  {2.0, 0.0, 0.0},
                  {3.0, 0.0, 0.0},
                  {4.0, 0.0, 0.0}};
  double rg = radius_of_gyration(chain, 5);
  /* The exact value for evenly spaced points 0..4 about the centroid 2 is
   * sqrt((4 + 1 + 0 + 1 + 4) / 5) = sqrt(2). */
  printf("Rg of a straight chain = %.6f (exact sqrt(2) = %.6f)\n", rg, sqrt(2.0));
  CRAFT_ASSERT(fabs(rg - sqrt(2.0)) < 1e-12);

  /* A translated copy: the raw RMSD is the translation distance, the
   * centroid-aligned RMSD is zero. */
  vec shifted[5];
  for (size_t i = 0; i < 5; ++i) {
    shifted[i] = (vec){chain[i].x + 10.0, chain[i].y, chain[i].z};
  }
  double raw = rmsd_raw(chain, shifted, 5);
  double aligned = rmsd_after_translation(chain, shifted, 5);
  printf("raw RMSD %.3f, after centroid alignment %.3f\n", raw, aligned);
  CRAFT_ASSERT(fabs(raw - 10.0) < 1e-12);
  CRAFT_ASSERT(aligned < 1e-12);
  CRAFT_ASSERT(aligned <= raw);

  /* A rotated copy: translation alone is not enough, so the aligned RMSD is
   * non-zero. Kabsch is what removes the rotation, and it is the reason the
   * full algorithm needs an SVD. */
  vec rotated[5];
  for (size_t i = 0; i < 5; ++i) {
    rotated[i] = (vec){-chain[i].y, chain[i].x, chain[i].z}; /* 90 degrees */
  }
  double rot_raw = rmsd_raw(chain, rotated, 5);
  double rot_aligned = rmsd_after_translation(chain, rotated, 5);
  printf("rotated: raw %.6f, translation only %.6f\n", rot_raw, rot_aligned);
  CRAFT_ASSERT(rot_raw > 1.0);       /* the frames differ */
  CRAFT_ASSERT(rot_aligned > 1e-9);  /* translation alone does not fix a rotation */

  /* The degenerate case: one point has no spread, so Rg is zero. */
  vec single[1] = {{3.0, 4.0, 5.0}};
  CRAFT_ASSERT(radius_of_gyration(single, 1) == 0.0);
  printf("single point Rg = 0\n");
  return 0;
}
