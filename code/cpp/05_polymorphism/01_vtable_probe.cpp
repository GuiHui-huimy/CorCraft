/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 05-01  polymorphism/01_vtable_probe.cpp
 * Drills: virtual dispatch, the vtable pointer inside the object, and the
 *         difference between calling through a base reference and by value.
 * Build:  cmake --build build --target cpp_05_vtable
 */
#include "craft_assert.h"

#include <iostream>
#include <memory>

class Shape {
public:
  virtual ~Shape() = default;
  virtual double area() const = 0;
  virtual const char *name() const { return "Shape"; }
};

class Square : public Shape {
public:
  explicit Square(double side) : side_(side) {}
  double area() const override { return side_ * side_; }
  const char *name() const override { return "Square"; }

private:
  double side_;
};

class Circle : public Shape {
public:
  explicit Circle(double radius) : radius_(radius) {}
  double area() const override { return 3.14159 * radius_ * radius_; }
  const char *name() const override { return "Circle"; }

private:
  double radius_;
};

/* A virtual call: the target is chosen at run time from the object's vtable. */
static double area_of(const Shape &s) { return s.area(); }

int main() {
  Square sq(2.0);
  Circle ci(1.0);

  CRAFT_ASSERT(area_of(sq) == 4.0);
  CRAFT_ASSERT(area_of(ci) > 3.14 && area_of(ci) < 3.15);
  std::cout << "Square area " << area_of(sq) << ", Circle area " << area_of(ci)
            << '\n';

  /* The vtable pointer is the first word of the object, which is why sizeof a
   * polymorphic class is at least a pointer larger than its data. */
  std::cout << "sizeof(Square)=" << sizeof(Square) << " (vptr + double)\n";
  CRAFT_ASSERT(sizeof(Square) >= sizeof(void *) + sizeof(double));

  /* Through a pointer: the dynamic type decides. Through a value: the static
   * type wins and the derived part is sliced away, which is the next lab. */
  Shape *p = &sq;
  CRAFT_ASSERT(p->area() == 4.0);
  CRAFT_ASSERT(std::string(p->name()) == "Square");

  /* The destructor must be virtual: deleting through unique_ptr<Shape> must
   * reach Circle's destructor. Without `virtual ~Shape()`, the delete below
   * would free the wrong size. */
  std::unique_ptr<Shape> owned = std::make_unique<Circle>(2.0);
  CRAFT_ASSERT(owned->area() > 12.5);
  std::cout << "owned " << owned->name() << " area " << owned->area() << '\n';

  std::cout << "sizeof(Shape)=" << sizeof(Shape) << " (abstract: no data)\n";
  return 0;
}
