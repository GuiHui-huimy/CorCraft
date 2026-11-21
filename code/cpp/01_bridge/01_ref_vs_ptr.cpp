/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 01-01  bridge/01_ref_vs_ptr.cpp
 * Drills: a reference is an alias, a pointer is an object; the four differences
 *         that decide which one a signature uses.
 * Build:  cmake --build build --target cpp_01_bridge
 */
#include "craft_assert.h"

#include <iostream>

struct Point {
  double x;
  double y;
};

/* By reference: no null, no reseat, no copy. The callee cannot tell whether it
 * received a named object or a temporary that was materialised for the call. */
static void scale_ref(Point &p, double factor) {
  p.x *= factor;
  p.y *= factor;
}

/* By pointer: the argument may be absent, so the callee must check. */
static bool scale_ptr(Point *p, double factor) {
  if (p == nullptr) {
    return false;
  }
  p->x *= factor;
  p->y *= factor;
  return true;
}

/* By const reference: no copy, no modification. This is the default for large
 * inputs and the reason a by-value parameter is a deliberate choice. */
static double norm_sq(const Point &p) { return p.x * p.x + p.y * p.y; }

int main() {
  Point p{3.0, 4.0};

  scale_ref(p, 2.0);
  CRAFT_ASSERT(p.x == 6.0);
  CRAFT_ASSERT(p.y == 8.0);

  CRAFT_ASSERT(scale_ptr(&p, 0.5));
  CRAFT_ASSERT(p.x == 3.0);

  CRAFT_ASSERT(!scale_ptr(nullptr, 2.0)); // the pointer's null case is explicit

  CRAFT_ASSERT(norm_sq(p) == 25.0);

  /* A reference cannot be rebound: after the initialisation, every assignment
   * through it writes the referent. A pointer can be reseated, which is the
   * capability a reference deliberately does not have. */
  Point q{1.0, 1.0};
  Point &alias = p;
  alias = q; // copies q into p; it does NOT make alias name q
  CRAFT_ASSERT(p.x == 1.0);
  CRAFT_ASSERT(q.x == 1.0);

  Point *ptr = &p;
  ptr = &q; // reseated: now names q
  CRAFT_ASSERT(ptr->x == 1.0);

  std::cout << "p=(" << p.x << ',' << p.y << ") q=(" << q.x << ',' << q.y
            << ")\n";
  std::cout << "sizeof(Point&) is not a thing; sizeof(Point*) = "
            << sizeof(Point *) << '\n';
  return 0;
}
