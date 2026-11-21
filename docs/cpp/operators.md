# Operator Overloading

An operator is a function with a strange name, and the first question is always whether it should be a member. I start with that question, then the invariant that an operator must preserve, then return types — reference for assignment, value for arithmetic — then the stream operators that must be non-members, then `<=>` and the rewrites it saves. The last section is the list of operators I would leave alone, with the reason for each.

An overloaded operator is a function with an unusual name. The name is chosen by the compiler from the syntax, the return type follows the usual rules, and the only genuinely new question is whether the function should be a member.

## 1. Member or non-member

| Operator | Usually | Why |
| --- | --- | --- |
| `+=`, `-=`, `[]`, `()` | member | it modifies or accesses the left operand |
| `+`, `-`, `*`, `/` | non-member, often `friend` | it returns a new object and treats both sides symmetrically |
| `==`, `<`, `<=>` | non-member | symmetry, and implicit conversion on both sides |
| `<<`, `>>` for streams | non-member | the left operand is a stream, not your type |

The rule behind the table is that a member function's left operand must already be of the class type. `a + 2` can be a member (`a.operator+(2)`), but `2 + a` cannot, unless a non-member overload exists. For symmetric arithmetic, the non-member form is what makes both orders work.

```cpp
// LAB 06-01  cpp_operators/01_fraction_ops.cpp
class Fraction {
public:
    Fraction(long num, long den);
    Fraction operator+(const Fraction &) const;      // member: left operand is *this
    Fraction &operator+=(const Fraction &);          // member: modifies *this
    friend Fraction operator*(Fraction, const Fraction &);  // symmetric
    ...
private:
    long num_, den_;
    void normalise();
};
```

## 2. The invariant is the whole class

A `Fraction` that does not reduce is a class that violates its own invariant, and every operator must restore it:

```cpp
Fraction::Fraction(long num, long den) : num_(num), den_(den) {
    if (den_ == 0) { throw std::invalid_argument("zero denominator"); }
    normalise();
}

void Fraction::normalise() {
    if (den_ < 0) { num_ = -num_; den_ = -den_; }        // keep the sign in the numerator
    long g = std::gcd(num_, den_);
    num_ /= g;
    den_ /= g;
}
```

Two decisions are embedded there. The sign is normalised into the numerator so that two equal fractions have identical representations, which makes `operator==` a comparison of two fields. And the denominator is checked in the constructor, so no `Fraction` object can exist in an invalid state — the invariant is established once rather than re-checked in every operator.

`std::gcd` is C++17 and lives in `<numeric>`. Before that, a hand-written Euclid loop was the standard.

## 3. Return types carry meaning

```cpp
Fraction operator+(const Fraction &rhs) const {          // returns a new value
    return Fraction(num_ * rhs.den_ + rhs.num_ * den_, den_ * rhs.den_);
}

Fraction &operator+=(const Fraction &rhs) {              // returns *this by reference
    *this = *this + rhs;
    return *this;
}
```

`+` returns by value because the result is a new object; `+=` returns by reference so that `a += b += c` works and so that the result can be used as an lvalue. Returning `const Fraction` from `+` was a 1990s fashion intended to prevent `a + b = c`; it also prevented moves and is now considered a defect.

## 4. Stream operators must be non-members

```cpp
friend std::ostream &operator<<(std::ostream &os, const Fraction &f) {
    return os << f.num_ << '/' << f.den_;
}
```

The left operand is `std::ostream`, so a member is impossible. The function returns the stream by reference so that chaining works: `os << a << b` is `operator<<(operator<<(os, a), b)`.

Making it a `friend` gives access to the private members without adding an accessor. The alternative — a public `num()` and `den()` — is also acceptable and is what the standard library style prefers.

## 5. Comparison, and `<=>`

```cpp
friend bool operator==(const Fraction &a, const Fraction &b) {
    return a.num_ == b.num_ && a.den_ == b.den_;
}

friend auto operator<=>(const Fraction &a, const Fraction &b) {
    return a.num_ * b.den_ <=> b.num_ * a.den_;      // C++20
}
```

The three-way comparison returns a comparison category, and the compiler derives `<`, `<=`, `>`, `>=` from it. Because the invariant keeps denominators positive and fractions reduced, comparing cross-multiplied numerators is exact — no floating point, no tolerance question.

Exact is not the same as safe. Cross multiplication multiplies two `long` values, and two values near 2^31 produce a product near 2^62, which is still inside a signed 64-bit integer but only by a factor of two — one more doubling and the comparison silently wraps into a negative number, so `3/4000000000 < 2/4000000000` becomes true. The lab's five-value table never comes close to that boundary, which is exactly how such a bug survives a test suite. For a general rational type I compare through `__int128` for the product, or compare by long division digit by digit and never materialise the product at all.

Pre-C++20 code writes each of the six operators by hand, which is six places to make the same mistake. The lab accepts either form and checks that all six agree on a table of values.

## 6. What not to overload

Three operators should be left alone in almost every codebase:

`&&`, `||` and `,` cannot be short-circuited or sequenced once overloaded, because the overload is an ordinary function call and both operands are evaluated. A class that overloads `&&` has silently changed the meaning of a very common expression.

`operator&` on a type that also has an address is confusing, and overloading `operator new`/`operator delete` for a class changes allocation globally for that type, which is a decision for a memory chapter, not a value-type chapter.

`operator T()` — an implicit conversion operator — is the one to be most careful with. It allows a class to be used where a `T` is expected, and a `bool` conversion in particular makes the object usable in every condition, including the ones where the author meant to compare.

```cpp
explicit operator bool() const;    // explicit: usable in if, not in int arithmetic
```

## 7. Field note: the division that was never reduced

The archived `fraction.h` implemented all four arithmetic operators and reduced none of them:

```cpp
fraction operator*(fraction beMul) {
    fraction result(0, 1);
    result.denominator = fraction::denominator * beMul.denominator;
    result.numerator = fraction::numerator * beMul.numerator;
    return result;                       // 2/3 * 3/2 is stored as 6/6
}
```

`2/3 * 3/2` printed as `6/6`. The arithmetic was correct and the representation was not, so `operator==` — had one existed — would have reported `6/6 != 1/1`. The class had an invariant in the author's head and nothing enforcing it in the object.

The repair is the `normalise()` call in the constructor, which makes the invariant structural. The lesson generalises: an invariant that is only maintained by the functions that happen to remember it is not an invariant, it is a hope.

## 8. Practise

Open `code/cpp/06_operators/01_fraction_ops.cpp`, which contains the unreduced version above, and make `labs/cpp/06_operators/` pass: reduction, sign normalisation, zero-denominator rejection, and all six comparisons agreeing on a five-value table.

## Continue

- [Templates](templates.md) lets the same arithmetic work for `int`, `long` and a rational type.
- [Construction and Lifetime](lifetime.md) explains why `+` returns by value.
- [STL Algorithms](algorithms.md) supplies comparators that replace operator overloads in generic code.
