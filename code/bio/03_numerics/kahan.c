/* LAB 03-01  numerics/kahan.c
 * Drills: how a naive sum loses the small terms, and how compensation keeps
 *         them, measured rather than asserted.
 * Build:  cmake --build build --target bio_03_kahan
 */
#include "craft_assert.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double sum_naive(const double *a, size_t n) {
  double s = 0.0;
  for (size_t i = 0; i < n; ++i) {
    s += a[i];
  }
  return s;
}

/* Kahan: the compensation term carries the low bits that the addition dropped. */
static double sum_kahan(const double *a, size_t n) {
  double sum = 0.0;
  double c = 0.0;
  for (size_t i = 0; i < n; ++i) {
    double y = a[i] - c;
    double t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }
  return sum;
}

/* The reference: long double has a wider mantissa on x86-64, so it is a
 * second opinion rather than a proof. */
static long double sum_extended(const double *a, size_t n) {
  long double s = 0.0L;
  for (size_t i = 0; i < n; ++i) {
    s += (long double)a[i];
  }
  return s;
}

int main(void) {
  const size_t n = 1000000;
  double *a = (double *)malloc(n * sizeof *a);
  CRAFT_ASSERT(a != NULL);

  /* The constructed case: one large term followed by many small ones. */
  a[0] = 1e16;
  for (size_t i = 1; i < n; ++i) {
    a[i] = 1.0;
  }

  double naive = sum_naive(a, n);
  double kahan = sum_kahan(a, n);
  long double reference = sum_extended(a, n);

  printf("naive     %.6f\n", naive);
  printf("kahan     %.6f\n", kahan);
  printf("extended  %.6Lf\n", reference);

  /* Naive loses the 999999 ones entirely: 1e16 + 1 is 1e16 in double. */
  CRAFT_ASSERT(naive == 1e16);
  CRAFT_ASSERT(kahan > naive);
  printf("naive lost %.0f of the small terms\n", (double)(reference - naive));

  /* The return to normal inputs: for similar magnitudes both are accurate, and
   * the compensation is unnecessary — which is the honest half of the claim. */
  for (size_t i = 0; i < n; ++i) {
    a[i] = 0.1;
  }
  double m_naive = sum_naive(a, n);
  double m_kahan = sum_kahan(a, n);
  printf("uniform 0.1: naive %.12f kahan %.12f mean %.12f\n", m_naive, m_kahan,
         (double)n * 0.1);

  free(a);
  return 0;
}
