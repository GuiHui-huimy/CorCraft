/* LAB 03-02  numerics/lse.c
 * Drills: the underflow in a naive sum of exponentials, the shift by the
 *         maximum that avoids it, and the comparison to a reference.
 * Build:  cmake --build build --target bio_03_lse
 */
#include "craft_assert.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

/* Naive: exp(-e) underflows to zero for e > 745 in double. */
static double naive_sum(const double *e, size_t n) {
  double s = 0.0;
  for (size_t i = 0; i < n; ++i) {
    s += exp(-e[i]);
  }
  return s;
}

/* log( sum exp(-e) ): factor out the minimum, so that the largest term becomes
 * exp(0) = 1 and nothing underflows.
 *
 *   sum exp(-e_i) = exp(-m) * sum exp(-(e_i - m))
 *   log sum exp(-e_i) = -m + log( sum exp(-(e_i - m)) )
 *
 * The sign of m is the part that is easy to get wrong: adding m instead of
 * subtracting it multiplies the answer by exp(2m). */
static double log_sum_exp(const double *e, size_t n) {
  if (n == 0) {
    return -INFINITY;
  }
  double m = e[0];
  for (size_t i = 1; i < n; ++i) {
    if (e[i] < m) {
      m = e[i];
    }
  }
  double s = 0.0;
  for (size_t i = 0; i < n; ++i) {
    s += exp(-(e[i] - m));
  }
  return -m + log(s);
}

int main(void) {
  /* Two energies: one representable, one that underflows on its own. */
  const double e[] = {1.0, 800.0};
  const size_t n = 2;

  double naive = naive_sum(e, n);
  double stable = log_sum_exp(e, n);

  printf("naive sum      %.17g\n", naive);
  printf("log-sum-exp    %.17g\n", stable);

  CRAFT_ASSERT(naive == exp(-1.0)); /* the second term vanished entirely */
  CRAFT_ASSERT(isfinite(stable));

  /* The relation: exp(log_sum) should equal the naive sum on inputs that do
   * not underflow, which checks the algebra rather than trusting it. */
  const double f[] = {1.0, 2.0, 3.0};
  double direct = naive_sum(f, 3);
  double via_log = exp(log_sum_exp(f, 3));
  double diff = direct - via_log;
  CRAFT_ASSERT(fabs(diff) < 1e-12 * (direct > 1.0 ? direct : 1.0));
  printf("equal on safe inputs: %.15f vs %.15f\n", direct, via_log);

  /* The clamp alternative is a defect: clamping the exponent changes the
   * answer rather than stabilising the computation. The value of the true
   * answer for {1, 800} is essentially exp(-1). */
  CRAFT_ASSERT(fabs(stable - (-1.0)) < 1e-12);
  printf("log-sum-exp of {1, 800} = %.15f, and the 800 contributes 0\n",
         stable);

  printf("exp(-745) = %.17g, exp(-746) = %.17g (underflow)\n", exp(-745.0),
         exp(-746.0));
  return 0;
}
