/* LAB 04-01  inheritance/01_layout_inherit.cpp
 * Drills: a base subobject is a member at offset zero; the empty base costs
 *         nothing; a non-virtual destructor deletes the wrong amount.
 * Build:  cmake --build build --target cpp_04_inherit
 */
#include "craft_assert.h"

#include <iostream>

struct Base {
  int b;
};

struct Derived : Base {
  int d;
};

/* Two bases, both non-empty: the second starts after the first. */
struct Left {
  int l;
};
struct Right {
  int r;
};
struct Both : Left, Right {
  int x;
};

/* An empty base is laid out at offset zero and occupies nothing, which is the
 * empty base optimisation that makes a policy class free. */
struct Empty {};
struct WithEmpty : Empty {
  int value;
};

/* The defect: deleting through a base pointer without a virtual destructor. */
struct NoVirtualDtor {
  int *data;
  explicit NoVirtualDtor() : data(new int[4]{1, 2, 3, 4}) {}
  ~NoVirtualDtor() { delete[] data; }
};

struct BadDerived : NoVirtualDtor {
  int extra = 42;
};

int main() {
  std::cout << "sizeof(Base)=" << sizeof(Base)
            << " sizeof(Derived)=" << sizeof(Derived) << '\n';
  CRAFT_ASSERT(sizeof(Derived) == sizeof(Base) + sizeof(int));
  CRAFT_ASSERT(sizeof(Both) == sizeof(int) * 3); // no padding between two ints

  std::cout << "sizeof(WithEmpty)=" << sizeof(WithEmpty)
            << " (empty base is free)\n";
  CRAFT_ASSERT(sizeof(WithEmpty) == sizeof(int));

  Derived d;
  d.b = 1;
  d.d = 2;
  /* A pointer to Derived converts to a pointer to Base, and the base subobject
   * is at offset zero, so the numeric value is unchanged. */
  Base *pb = &d;
  CRAFT_ASSERT(pb == reinterpret_cast<Base *>(&d));
  CRAFT_ASSERT(pb->b == 1);
  std::cout << "base at " << static_cast<void *>(pb) << ", derived at "
            << static_cast<void *>(&d) << '\n';

  /* The slicing case: assigning a Derived to a Base copies only the Base part. */
  Base sliced = d;
  CRAFT_ASSERT(sliced.b == 1);

  /* The deletion case is described rather than run, because running it on a
   * heap object would be undefined behaviour and the sanitizer reports a
   * type-mismatched delete. The sizes are the evidence. */
  BadDerived on_stack;
  NoVirtualDtor *base = &on_stack;
  std::cout << "sizeof(NoVirtualDtor)=" << sizeof(NoVirtualDtor)
            << " sizeof(BadDerived)=" << sizeof(BadDerived)
            << ": a delete through the base frees the smaller size\n";
  (void)base;

  std::cout << "rule: a polymorphic base needs a virtual destructor\n";
  return 0;
}
