/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 06-01  operators/01_fraction_ops.cpp
 * Drills: an invariant maintained by the constructor, the member versus
 *         non-member decision, and the stream operator that must be a friend.
 * Build:  cmake --build build --target cpp_06_fraction
 */
#include "craft_assert.h"

#include <iostream>
#include <numeric>
#include <stdexcept>

class Fraction {
public:
  Fraction(long num, long den) : num_(num), den_(den) {
    if (den == 0) {
      throw std::invalid_argument("zero denominator");
    }
    long g = std::gcd(num, den); // 6/8 becomes 3/4 at construction
    num_ /= g;
    den_ /= g;
    if (den_ < 0) { // the sign lives in the numerator
      num_ = -num_;
      den_ = -den_;
    }
  }

  long num() const { return num_; }
  long den() const { return den_; }

  /* Compound assignment as a member: it modifies *this, so it must be a member
   * and must return *this by reference to chain. */
  Fraction &operator+=(const Fraction &rhs) {
    long n = num_ * rhs.den_ + rhs.num_ * den_;
    long d = den_ * rhs.den_;
    *this = Fraction(n, d); // the constructor re-establishes the invariant
    return *this;
  }

  /* The stream operator cannot be a member: the left operand is std::ostream,
   * not Fraction. A friend defined inline keeps it in the class's scope. */
  friend std::ostream &operator<<(std::ostream &out, const Fraction &f) {
    return out << f.num_ << '/' << f.den_;
  }

  friend bool operator==(const Fraction &a, const Fraction &b) {
    return a.num_ == b.num_ && a.den_ == b.den_; // reduced form makes this exact
  }

private:
  long num_;
  long den_;
};

/* Non-member, non-friend: uses only the public interface, which is what makes
 * it the better choice when it can be written that way. */
static Fraction operator+(Fraction a, const Fraction &b) {
  a += b; // reuses the member and takes the copy it needs
  return a;
}

int main() {
  Fraction a(1, 2);
  Fraction b(1, 3);
  Fraction c = a + b;

  CRAFT_ASSERT(c.num() == 5);
  CRAFT_ASSERT(c.den() == 6);
  std::cout << "1/2 + 1/3 = " << c << '\n';

  /* The defect the constructor prevents: unreduced results compare unequal
   * even when their values are equal. 2/3 * 3/2 is 1, but 6/6 != 1/1 unless
   * both are reduced. */
  Fraction d(2, 3);
  Fraction e(3, 2);
  Fraction product(d.num() * e.num(), d.den() * e.den());
  CRAFT_ASSERT(product == Fraction(1, 1));
  std::cout << "2/3 * 3/2 = " << product << '\n';

  a += b;
  CRAFT_ASSERT(a == Fraction(5, 6));
  std::cout << "a after += b: " << a << '\n';

  bool threw = false;
  try {
    Fraction bad(1, 0);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  CRAFT_ASSERT(threw);

  /* The sign is normalised: 1/-2 and -1/2 become the same object. */
  CRAFT_ASSERT(Fraction(1, -2) == Fraction(-1, 2));
  std::cout << "1/-2 normalises to " << Fraction(1, -2) << '\n';
  return 0;
}
