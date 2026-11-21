/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 06-01  geometry/dihedral.c
 * Drills: the four-atom dihedral with atan2, the clamp that avoids a NaN, and
 *         a hand-computable case.
 * Build:  cmake --build build --target bio_06_dihedral
 */
#include "craft_assert.h"

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
  double x, y, z;
} vec;

static vec sub(vec a, vec b) { return (vec){a.x - b.x, a.y - b.y, a.z - b.z}; }
static double dot(vec a, vec b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static vec cross(vec a, vec b) {
  return (vec){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
               a.x * b.y - a.y * b.x};
}
static double norm(vec a) { return sqrt(dot(a, a)); }
static vec unit(vec a) {
  double n = norm(a);
  return (vec){a.x / n, a.y / n, a.z / n};
}

/* atan2 rather than acos: the cosine alone cannot distinguish the sign, and a
 * peptide's dihedral sign is the difference between two conformations.
 * The bond vectors follow the standard convention b1 = r2 - r1, b2 = r3 - r2,
 * b3 = r4 - r3; reversing b1 shifts every result by 180 degrees. */
static double dihedral(vec p1, vec p2, vec p3, vec p4) {
  vec b1 = sub(p2, p1);
  vec b2 = sub(p3, p2);
  vec b3 = sub(p4, p3);
  vec n1 = cross(b1, b2);
  vec n2 = cross(b2, b3);
  double y = dot(cross(n1, n2), unit(b2));
  double x = dot(n1, n2);
  return atan2(y, x) * 180.0 / M_PI;
}

/* The clamp that prevents acos(NaN): the computed cosine of a nearly straight
 * angle can exceed 1 by one ULP, and acos(1.0000000000000002) is NaN. */
static double angle(vec p1, vec p2, vec p3) {
  vec u = sub(p1, p2);
  vec v = sub(p3, p2);
  double c = dot(u, v) / (norm(u) * norm(v));
  if (c > 1.0) {
    c = 1.0;
  }
  if (c < -1.0) {
    c = -1.0;
  }
  return acos(c) * 180.0 / M_PI;
}

int main(void) {
  /* A planar trans arrangement: four points in the z=0 plane with a straight
   * chain give a dihedral of 180 (or -180). */
  vec a = {0.0, 0.0, 0.0};
  vec b = {1.0, 0.0, 0.0};
  vec c = {1.0, 1.0, 0.0};
  vec d = {2.0, 1.0, 0.0};

  double phi = dihedral(a, b, c, d);
  printf("planar dihedral %.6f\n", phi);
  CRAFT_ASSERT(fabs(fabs(phi) - 180.0) < 1e-9);

  /* Rotating d out of the plane changes the sign: the same magnitude, the
   * opposite sign. That is what atan2 provides and acos cannot. */
  vec up = {2.0, 1.0, 0.5};
  vec down = {2.0, 1.0, -0.5};
  double du = dihedral(a, b, c, up);
  double dd = dihedral(a, b, c, down);
  printf("out of plane: %.6f and %.6f\n", du, dd);
  CRAFT_ASSERT(fabs(du + dd) < 1e-9);
  CRAFT_ASSERT(isfinite(du) && isfinite(dd));

  /* The angle of a straight arrangement is 180 and must not be NaN. */
  vec straight = {2.0, 0.0, 0.0};
  double theta = angle(a, b, straight);
  printf("straight angle %.6f\n", theta);
  CRAFT_ASSERT(isfinite(theta));
  CRAFT_ASSERT(fabs(theta - 180.0) < 1e-9);

  /* A right angle: (0,0,0), (1,0,0), (1,1,0) at b gives 90. */
  vec corner = {1.0, 1.0, 0.0};
  double right = angle(a, b, corner);
  printf("right angle %.6f\n", right);
  CRAFT_ASSERT(fabs(right - 90.0) < 1e-9);

  /* The NaN demonstration: acos of a value just above 1. The 1e-15 must exceed
   * one ULP of 1.0 (about 2.2e-16), otherwise the sum rounds back to exactly 1
   * and acos returns 0 rather than NaN. */
  double over = 1.0 + 1e-15;
  double bad = acos(over);
  printf("acos(1 + 1e-15) is NaN: %d\n", isnan(bad));
  CRAFT_ASSERT(isnan(bad));

  printf("degrees per radian %.15f\n", 180.0 / M_PI);
  return 0;
}
