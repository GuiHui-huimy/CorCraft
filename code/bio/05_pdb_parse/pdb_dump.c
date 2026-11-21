/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 05-01  pdb_parse/pdb_dump.c
 * Drills: fixed-column parsing, the chain field that whitespace splitting
 *         loses, and the hierarchy a single pass must build.
 * Build:  cmake --build build --target bio_05_pdb
 *
 * The test records are built with the format's own column widths instead of
 * being typed by hand: a hand-typed line that is one column off would test the
 * typo rather than the parser.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The PDB column layout, as a format string:
 *   1-6 record, 7-11 serial, 13-16 atom name, 17 altLoc, 18-20 resName,
 *   22 chainID, 23-26 resSeq, 27 iCode, 31-38 x, 39-46 y, 47-54 z,
 *   55-60 occupancy, 61-66 tempFactor, 77-78 element. */
static void make_atom(char *out, size_t cap, const char *record, int serial,
                      const char *name, char altloc, const char *res_name,
                      char chain, int seq, char icode, double x, double y,
                      double z, const char *element) {
  snprintf(out, cap, "%-6s%5d %-4s%c%3s %c%4d%c   %8.3f%8.3f%8.3f%6.2f%6.2f          %2s",
           record, serial, name, altloc, res_name, chain, seq, icode, x, y, z,
           1.00, 9.19, element);
}

typedef struct {
  size_t atoms;
  size_t hetatms;
  size_t chains;
  size_t residues;
} counters;

static void field(const char *line, size_t offset, size_t width, char *out,
                  size_t out_cap) {
  size_t n = width < out_cap - 1 ? width : out_cap - 1;
  memcpy(out, line + offset, n);
  out[n] = '\0';
}

/* One pass. The residue key is (chain, resSeq, iCode), not resSeq alone: two
 * chains can share a sequence number, and an insertion code can distinguish two
 * residues in one chain. */
static void count(const char *text, counters *c) {
  memset(c, 0, sizeof *c);
  char last_chain = '\0';
  int last_seq = 0;
  char last_icode = '\0';
  int have_last = 0;

  const char *line = text;
  while (*line != '\0') {
    size_t len = 0;
    while (line[len] != '\n' && line[len] != '\0') {
      ++len;
    }
    int is_atom = (len >= 54 && strncmp(line, "ATOM  ", 6) == 0);
    int is_het = (len >= 54 && strncmp(line, "HETATM", 6) == 0);
    if (is_atom) {
      ++c->atoms;
    }
    if (is_het) {
      ++c->hetatms;
    }
    if (is_atom || is_het) {
      char chain = line[21]; /* column 22 */
      char seqbuf[5];
      field(line, 22, 4, seqbuf, sizeof seqbuf); /* columns 23-26 */
      int seq = atoi(seqbuf);
      char icode = line[26]; /* column 27 */

      /* The old chain is compared before it is overwritten: updating last_chain
       * first made the residue test below always false for a new chain. */
      int new_chain = (!have_last) || (chain != last_chain);
      int new_residue =
          new_chain || (seq != last_seq) || (icode != last_icode);

      if (new_chain) {
        ++c->chains;
      }
      if (new_residue) {
        ++c->residues;
      }

      have_last = 1;
      last_chain = chain;
      last_seq = seq;
      last_icode = icode;
    }
    line += len;
    if (*line == '\n') {
      ++line;
    }
  }
}

int main(void) {
  char buf[6][200];
  size_t n = 0;

  make_atom(buf[n++], 200, "ATOM  ", 1, " N  ", ' ', "MET", 'A', 1, ' ', 27.340,
            24.430, 2.614, " N");
  make_atom(buf[n++], 200, "ATOM  ", 2, " CA ", ' ', "MET", 'A', 1, ' ', 26.266,
            25.413, 2.842, " C");
  make_atom(buf[n++], 200, "ATOM  ", 3, " C  ", ' ', "MET", 'A', 1, ' ', 26.913,
            26.639, 2.332, " C");
  make_atom(buf[n++], 200, "ATOM  ", 4, " N  ", ' ', "GLN", 'A', 2, ' ', 28.000,
            25.000, 3.000, " N");
  make_atom(buf[n++], 200, "HETATM", 5, " O  ", ' ', "HOH", 'A', 101, ' ',
            30.000, 26.000, 4.000, " O");

  char text[1200];
  size_t k = 0;
  for (size_t i = 0; i < n; ++i) {
    size_t len = strlen(buf[i]);
    memcpy(text + k, buf[i], len);
    k += len;
    text[k++] = '\n';
  }
  text[k] = '\0';

  /* The columns are what make the parse correct: check one field directly. */
  CRAFT_ASSERT(strncmp(buf[0], "ATOM  ", 6) == 0);
  CRAFT_ASSERT(buf[0][21] == 'A'); /* chain, column 22 */
  CRAFT_ASSERT(buf[0][26] == ' '); /* iCode, column 27 */

  counters c;
  count(text, &c);
  printf("atoms %zu hetatms %zu chains %zu residues %zu\n", c.atoms, c.hetatms,
         c.chains, c.residues);
  CRAFT_ASSERT(c.atoms == 4);
  CRAFT_ASSERT(c.hetatms == 1);
  CRAFT_ASSERT(c.chains == 1);
  CRAFT_ASSERT(c.residues == 3); /* MET 1, GLN 2, HOH 101 */

  /* Coordinates by offset: a whitespace split would lose the right alignment. */
  char xbuf[9], ybuf[9], zbuf[9];
  field(buf[0], 30, 8, xbuf, sizeof xbuf);
  field(buf[0], 38, 8, ybuf, sizeof ybuf);
  field(buf[0], 46, 8, zbuf, sizeof zbuf);
  printf("coordinates %s %s %s\n", xbuf, ybuf, zbuf);
  CRAFT_ASSERT(atof(xbuf) == 27.34);
  CRAFT_ASSERT(atof(zbuf) == 2.614);

  /* The atom name keeps its leading space: that alignment is the only signal
   * distinguishing a calcium CA from an alpha carbon. */
  char atom_name[5];
  field(buf[0], 12, 4, atom_name, sizeof atom_name);
  printf("atom name field '[%s]'\n", atom_name);
  CRAFT_ASSERT(atom_name[0] == ' ');

  /* Two chains sharing a resSeq: a parser keyed on resSeq merges them, and the
   * chain count is what shows it. */
  char m1[200], m2[200], multi[500];
  make_atom(m1, 200, "ATOM  ", 1, " N  ", ' ', "ALA", 'A', 1, ' ', 1.0, 2.0, 3.0,
            " N");
  make_atom(m2, 200, "ATOM  ", 2, " N  ", ' ', "ALA", 'B', 1, ' ', 1.0, 2.0, 4.0,
            " N");
  snprintf(multi, sizeof multi, "%s\n%s\n", m1, m2);

  counters mc;
  count(multi, &mc);
  CRAFT_ASSERT(mc.chains == 2);
  CRAFT_ASSERT(mc.residues == 2);
  printf("two chains, same resSeq: chains %zu residues %zu\n", mc.chains,
         mc.residues);
  return 0;
}
