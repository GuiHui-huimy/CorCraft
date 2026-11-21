/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 09-01  parallel/omp_scores.c
 * Drills: the scoring loop distributed with OpenMP, wall time measured instead
 *         of CPU time, a reduction that changes the last digits, and the
 *         serial fallback when OpenMP is not available.
 * Build:  cmake --build build --target bio_09_parallel
 *
 * The serial path is what runs when -fopenmp is not in the compile line, so the
 * lab always builds and always has a reference answer.
 */
#include "craft_assert.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define N_LIGANDS 2048
#define N_ATOMS 256
#define CUTOFF2 (36.0) /* 6 Angstrom, squared */

typedef struct {
  double x, y, z;
} vec;

/* A deterministic generator: the same ligands on every machine, so a score can
 * be compared across runs and across thread counts. */
static unsigned int lcg(unsigned int seed) {
  return seed * 1103515245u + 12345u;
}

static void fill_atoms(vec *atoms) {
  unsigned int seed = 7u;
  for (size_t i = 0; i < N_ATOMS; ++i) {
    seed = lcg(seed);
    atoms[i].x = (double)(seed % 1000u) / 100.0;
    seed = lcg(seed);
    atoms[i].y = (double)(seed % 1000u) / 100.0;
    seed = lcg(seed);
    atoms[i].z = (double)(seed % 1000u) / 100.0;
  }
}

static void fill_ligand(vec *ligand, int id) {
  unsigned int seed = (unsigned int)(id + 1);
  for (size_t i = 0; i < 8; ++i) {
    seed = lcg(seed);
    ligand[i].x = (double)(seed % 1000u) / 100.0;
    seed = lcg(seed);
    ligand[i].y = (double)(seed % 1000u) / 100.0;
    seed = lcg(seed);
    ligand[i].z = (double)(seed % 1000u) / 100.0;
  }
}

/* The score: number of ligand atoms within the cutoff of any protein atom.
 * Reads the protein, writes nothing: the iterations are independent. */
static double score_ligand(const vec *ligand, const vec *protein) {
  int contacts = 0;
  for (size_t i = 0; i < 8; ++i) {
    for (size_t j = 0; j < N_ATOMS; ++j) {
      double dx = ligand[i].x - protein[j].x;
      double dy = ligand[i].y - protein[j].y;
      double dz = ligand[i].z - protein[j].z;
      if (dx * dx + dy * dy + dz * dz < CUTOFF2) {
        ++contacts;
      }
    }
  }
  return (double)contacts;
}

/* Wall time, not CPU time: clock() sums every thread's time and reports a
 * parallel run as slower than a serial one. timespec_get is C11 and needs no
 * POSIX feature macro. */
static double wall_seconds(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(void) {
  vec *protein = (vec *)malloc(N_ATOMS * sizeof *protein);
  vec *ligands = (vec *)malloc((size_t)N_LIGANDS * 8 * sizeof *ligands);
  CRAFT_ASSERT(protein != NULL && ligands != NULL);

  fill_atoms(protein);
  for (int i = 0; i < N_LIGANDS; ++i) {
    fill_ligand(ligands + (size_t)i * 8, i);
  }

  /* The serial reference is computed once and kept: every parallel run must
   * agree with it to a tolerance that the reduction's order makes necessary. */
  double serial = 0.0;
  double t0 = wall_seconds();
  for (int i = 0; i < N_LIGANDS; ++i) {
    serial += score_ligand(ligands + (size_t)i * 8, protein);
  }
  double t1 = wall_seconds();
  printf("serial   total %.3f  %.4f s\n", serial, t1 - t0);

  double parallel = 0.0;
  double t2 = wall_seconds();
#ifdef _OPENMP
  int threads = 1;
#pragma omp parallel
  {
#pragma omp single
    threads = omp_get_num_threads();
  }
#pragma omp parallel for reduction(+ : parallel) schedule(static)
  for (int i = 0; i < N_LIGANDS; ++i) {
    parallel += score_ligand(ligands + (size_t)i * 8, protein);
  }
  double t3 = wall_seconds();
  printf("openmp   total %.3f  %.4f s  threads %d  speedup %.2fx\n",
         parallel, t3 - t2, threads, (t1 - t0) / (t3 - t2));
#else
  parallel = serial;
  double t3 = wall_seconds();
  printf("openmp   not enabled: serial fallback %.4f s\n", t3 - t2);
#endif

  /* The tolerance, not equality: floating-point addition is not associative,
   * so a reduction over several threads reorders the sum. */
  double diff = parallel - serial;
  double scale = serial > 1.0 ? serial : 1.0;
  CRAFT_ASSERT(fabs(diff) <= 1e-9 * scale);
  printf("|parallel - serial| = %.3e (within 1e-9 relative)\n", fabs(diff));

  printf("ligands %d, protein atoms %d, %zu score evaluations\n", N_LIGANDS,
         N_ATOMS, (size_t)N_LIGANDS * 8 * N_ATOMS);

  free(protein);
  free(ligands);
  return 0;
}
