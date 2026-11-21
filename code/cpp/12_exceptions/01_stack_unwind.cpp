/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 12-01  exceptions/01_stack_unwind.cpp
 * Drills: unwinding destroys what was constructed, a raw pointer leaks, a
 *         constructor that throws releases its members, and noexcept changes
 *         what the compiler may do.
 * Build:  cmake --build build --target cpp_12_exceptions
 */
#include "craft_assert.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Tracer {
  static int alive;
  int id;
  explicit Tracer(int v) : id(v) {
    ++alive;
    std::cout << "Tracer(" << id << ") alive=" << alive << '\n';
  }
  Tracer(const Tracer &other) : id(other.id) {
    ++alive;
    std::cout << "Tracer copy alive=" << alive << '\n';
  }
  ~Tracer() {
    --alive;
    std::cout << "~Tracer(" << id << ") alive=" << alive << '\n';
  }
};

int Tracer::alive = 0;

/* One RAII member and one raw pointer whose destructor frees it: unwinding runs
 * both, and the printed order is the lesson. Delete the destructor and ASan
 * reports the block as leaked at exit — which is why ownership belongs in a
 * type, where the compiler enforces it, and not in a comment. */
struct Mixed {
  Tracer tracer;
  int *raw;
  explicit Mixed(int v) : tracer(v), raw(new int[8]) {}
  ~Mixed() { delete[] raw; }
};

static void throws_after_alloc(int *leak_flag) {
  Mixed m(1);
  *leak_flag = 1;
  throw std::runtime_error("boom"); // ~Mixed runs: tracer destroyed, raw freed
}

/* A constructor that throws after acquiring one resource and before acquiring
 * the second: the first member is destroyed because it was constructed. */
struct TwoResources {
  Tracer first;
  Tracer second;
  TwoResources(bool fail) : first(1), second(2) {
    if (fail) {
      throw std::runtime_error("second failed");
    }
  }
};

int main() {
  int reached = 0;
  try {
    throws_after_alloc(&reached);
  } catch (const std::runtime_error &e) {
    CRAFT_ASSERT(reached == 1);
    std::cout << "caught: " << e.what() << '\n';
  }
  CRAFT_ASSERT(Tracer::alive == 0); // no Tracer outlived the throw

  try {
    TwoResources bad(true);
    (void)bad; // unreachable: the constructor throws, so this line never runs
  } catch (const std::runtime_error &) {
    std::cout << "constructor threw after first was built\n";
  }
  CRAFT_ASSERT(Tracer::alive == 0);

  /* Catching by reference preserves the dynamic type; catching by value would
   * slice it. The handler must name a base this exception actually derives
   * from: std::out_of_range derives from std::logic_error, not from
   * std::runtime_error, so a runtime_error handler would not catch it and the
   * exception would reach std::terminate. */
  try {
    throw std::out_of_range("index");
  } catch (const std::logic_error &e) {
    CRAFT_ASSERT(std::string(e.what()) == "index");
    std::cout << "caught by reference: " << e.what() << '\n';
  }

  /* A vector of a noexcept-movable type reallocates by moving. */
  std::vector<Tracer> v;
  v.reserve(2);
  v.emplace_back(10);
  v.emplace_back(11);
  CRAFT_ASSERT(v.size() == 2);
  std::cout << "vector holds " << v.size() << " Tracers\n";

  CRAFT_ASSERT(Tracer::alive == 2);

  bool caught_all = false;
  try {
    throw 42; // not a std::exception
  } catch (...) {
    caught_all = true;
  }
  CRAFT_ASSERT(caught_all);

  v.clear();
  CRAFT_ASSERT(Tracer::alive == 0);
  return 0;
}
