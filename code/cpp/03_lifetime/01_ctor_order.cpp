/* LAB 03-01  lifetime/01_ctor_order.cpp
 * Drills: members are constructed in declaration order, not in initialiser
 *         order; the copies a by-value parameter and a return each cost.
 * Build:  cmake --build build --target cpp_03_ctor
 */
#include "craft_assert.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Trace {
  static int constructed;
  static int copied;
  static int moved;

  int id;

  explicit Trace(int v) : id(v) { ++constructed; }
  Trace(const Trace &other) : id(other.id) {
    ++copied;
    ++constructed;
  }
  Trace(Trace &&other) noexcept : id(other.id) {
    other.id = -1;
    ++moved;
    ++constructed;
  }
  Trace &operator=(const Trace &) = default;
  Trace &operator=(Trace &&) = default;
  ~Trace() = default;
};

int Trace::constructed = 0;
int Trace::copied = 0;
int Trace::moved = 0;

/* Declaration order decides construction order. The initialiser list order is
 * irrelevant and the compiler warns when the two disagree. */
class Ordered {
public:
  Ordered() : second_(1), first_(2) {} // second_ is declared first, so it wins
  int first_value() const { return first_.id; }
  int second_value() const { return second_.id; }

private:
  Trace second_; // constructed first
  Trace first_;  // constructed second
};

static Trace by_value(Trace t) { return t; }               // copy in, move out
static Trace by_ref(const Trace &t) { return t; }          // copy out only
static void by_const_ref(const Trace &t) { (void)t; }      // nothing

int main() {
  Trace::constructed = Trace::copied = Trace::moved = 0;
  Ordered o;
  CRAFT_ASSERT(o.second_value() == 1);
  CRAFT_ASSERT(o.first_value() == 2);
  std::cout << "constructed by Ordered: " << Trace::constructed << '\n';

  Trace::constructed = Trace::copied = Trace::moved = 0;
  Trace source(7);
  Trace copy_in = by_value(source);
  CRAFT_ASSERT(copy_in.id == 7);
  std::cout << "by_value:     constructed=" << Trace::constructed
            << " copied=" << Trace::copied << " moved=" << Trace::moved << '\n';

  Trace::constructed = Trace::copied = Trace::moved = 0;
  Trace copy_out = by_ref(source);
  CRAFT_ASSERT(copy_out.id == 7);
  std::cout << "by_ref:       constructed=" << Trace::constructed
            << " copied=" << Trace::copied << " moved=" << Trace::moved << '\n';

  Trace::constructed = Trace::copied = Trace::moved = 0;
  by_const_ref(source);
  CRAFT_ASSERT(Trace::constructed == 0);
  std::cout << "by_const_ref: constructed=0 (no copy at all)\n";

  /* The vector copies on growth unless the element's move constructor is
   * noexcept. Trace's is, so a reallocation moves. */
  Trace::constructed = Trace::copied = Trace::moved = 0;
  std::vector<Trace> v;
  v.reserve(1);
  v.emplace_back(1);
  v.emplace_back(2);
  v.emplace_back(3);
  std::cout << "vector growth: copied=" << Trace::copied
            << " moved=" << Trace::moved << '\n';
  return 0;
}
