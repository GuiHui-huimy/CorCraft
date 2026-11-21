# Numerics for Scoring

Floating point is not a detail you can postpone in structure work, because a sum of 10^5 terms drifts and a log-sum-exp overflows. I show the drift, then Kahan summation, then log-sum-exp with the minimum pulled out rather than the maximum, then relative error and the comparison built on it, then summation order as a semantic choice rather than housekeeping.

A scoring function sums thousands of small terms. The individual terms are computed to about 16 digits; the sum, accumulated naively, may be correct to 12. That difference is invisible until it changes a decision, and in structural biology a score comparison is a decision.

This page is the floating-point behaviour the scoring chapters depend on, kept to what a scoring loop actually needs.

## 1. Why the sum drifts

```c
/* LAB 03-01  bio_numerics/kahan.c */
double s = 0.0;
for (size_t i = 0; i < n; ++i) { s += a[i]; }
```

Each addition rounds to the nearest representable double. The rounding error is bounded by half an ULP of the current partial sum, so it grows with the magnitude of the partial sum, not with the number of terms. When the terms are of similar size the errors partially cancel and the result is good; when one term is much larger than the rest, every later addition loses the low bits of the small terms.

The pathological case is a sum where a large prefix is followed by many small terms. In a scoring loop this happens whenever a distance term dominates and the remaining terms are corrections.

```c
double s = 0.0;
for (i) { s += 1e16; }
s += 1.0;      /* the 1.0 is lost: 1e16 + 1 is 1e16 in double */
```

The mechanism: `double` has 53 bits of mantissa, so `1e16` is about \(2^{53}\), and adding `1` needs a bit below the last representable place. The result is exactly `1e16`.

## 2. Kahan summation

The compensation term carries the lost low bits:

```c
double sum = 0.0, c = 0.0;
for (size_t i = 0; i < n; ++i) {
    double y = a[i] - c;
    double t = sum + y;
    c = (t - sum) - y;
    sum = t;
}
```

Four operations instead of one, and the error bound drops from \(O(n \epsilon)\) to \(O(\epsilon)\) independent of n for well-behaved inputs. The cost is the four operations and one extra register.

Whether it is worth it: for a 76-residue protein, no — the naive sum is already accurate to 15 digits. For a 10⁶-term scoring pass over a ligand library, yes, because a systematic bias of \(10^{-12}\) in a score used as a filter changes the set of molecules that pass.

The lab measures the difference rather than asserting it: it runs both sums on a constructed vector where the naive version is known to lose, and prints both results next to the exact answer computed with `long double` or with a rational reference.

## 3. Log-sum-exp, and why scores are not probabilities

A scoring function often returns an energy, and a probability is \(e^{-E/kT}\). Summing probabilities of many states underflows: `exp(-700)` is zero in double, while the individual energies are perfectly representable.

```c
/* the naive version: underflows to 0 */
double p = 0.0;
for (i) { p += exp(-e[i]); }

/* the stable version: factor out the minimum */
double m = min(e, n);
double s = 0.0;
for (i) { s += exp(-(e[i] - m)); }
double log_sum = -m + log(s);       /* log of the sum of exp(-e[i]) */
```

The shift is by the **minimum**, because the terms are `exp(-e[i])`: subtracting the smallest `e` makes the largest term `exp(0) = 1` and every other term at most one, so the sum lands in `[1, n]` and can neither underflow nor overflow. The result is recovered by subtracting `m` again after the logarithm. Factoring out the maximum instead would leave the largest term at `exp(-(max - min))`, which underflows as soon as the range exceeds about 700 — the exact failure the shift was meant to prevent.

The cost is one pass to find the minimum and one pass to sum, which is why a scoring loop that needs a normalised probability pays for two passes. The alternative — clamping the exponent — is a defect: it changes the answer rather than stabilising the computation.

## 4. Relative error, and how to compare two doubles

```c
static int nearly_equal(double a, double b) {
    double d = fabs(a - b);
    double m = fmax(fabs(a), fabs(b));
    return d <= 1e-12 * (m > 1.0 ? m : 1.0);
}
```

An absolute tolerance is wrong for large values (it demands impossible precision) and wrong for tiny values (it accepts garbage). The relative form with a floor at 1.0 is the standard compromise, and it is the form the labs' assertions use.

The floor matters: for `a = 1e-18` and `b = 2e-18`, the relative difference is 100%, but both are noise near zero and the comparison should pass or fail on a project-specific absolute threshold, not on this formula.

## 5. Summation order is not a detail

Two cores summing the same array in a different order produce different results, because floating-point addition is not associative.

```c
(a + b) + c    !=    a + (b + c)
```

This is the reason a parallel reduction must be deterministic-by-construction if its output is compared across runs: either fix the partition and the order (which OpenMP's `reduction(+:sum)` does within a schedule, not across thread counts), or accept that the score varies in the last digits and write the tolerance accordingly.

The parallel-scoring chapter states its rule explicitly: the serial and parallel runs must agree to `1e-9`, not bit-for-bit, and the reason is this paragraph.

## 6. Field note: the fraction that was six sixths

The archived `fraction.h` stored a fraction as two integers and multiplied without reducing:

```cpp
Fraction operator*(const Fraction &a, const Fraction &b) {
    return Fraction(a.num * b.num, a.den * b.den);   // no reduction
}
```

`2/3 * 3/2` produced `6/6`, which is numerically 1 and structurally not. A comparison against `Fraction(1, 1)` failed, and a sum of ten multiplications overflowed an `int` denominator before the error became visible.

The repair is to reduce in the constructor, so every path that constructs a `Fraction` produces a canonical form:

```cpp
Fraction(long n, long d) : num(n), den(d) {
    long g = std::gcd(n, d);
    num /= g; den /= g;
    if (den < 0) { num = -num; den = -den; }
}
```

This is the invariant-as-constructor argument from the operators chapter, and it is the same argument as this page's: a numeric type that does not maintain its own representation invites every caller to fix it differently. The generalisation to floating point is that a sum which does not control its own error accumulation invites every caller to distrust the result.

## 7. Practise

Run `code/bio/03_numerics/kahan.c` and `lse.c`, and confirm that the naive sum's error grows with n while the compensated one's does not. `lse.c` asserts exactly why the shift is needed: the naive version returns `exp(-1.0)` because the second term underflows to zero, while the stable version matches the direct computation to `1e-12` relative. Attempt the exercise in `labs/bio/03_numerics/`: reimplement the shifted form without the sign mistake, and check it on an input whose exponent range exceeds 700.

## Continue

- [Alignment](alignment.md) is where these sums appear in a real scoring loop.
- [Parallel Scoring](parallel-scoring.md) is where summation order becomes a reproducibility question.
- [Undefined Behaviour](../c/undefined-behavior.md) covers the integer side of the same arithmetic.
