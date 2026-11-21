# Contacts

A contact map is a definition with three decisions inside it, and the cost is quadratic until you stop comparing everything with everything. I fix the definition first, then measure the naive loop, then bin spatially, then sweep sorted coordinates, then store the map in a form that does not waste half its bytes on symmetry.

A contact map is an n×n boolean matrix: cell (i, j) is true when residues i and j are within a cutoff distance. It is the simplest structural descriptor that captures something real, and the cheapest place in this book to see the difference between an algorithm that scales and one that does not.

## 1. The definition, and the three decisions inside it

```c
/* LAB 07-01  bio_contacts/contact_map.c */
#define CUTOFF 8.0
#define CUTOFF2 (CUTOFF * CUTOFF)

int residue_contact(const residue *a, const residue *b) {
    for (size_t i = 0; i < a->n_atoms; ++i) {
        for (size_t j = 0; j < b->n_atoms; ++j) {
            if (dist2(&a->atoms[i], &b->atoms[j]) < CUTOFF2) { return 1; }
        }
    }
    return 0;
}
```

Three decisions that a paper must state and a program must choose:

1. **Which atoms.** All heavy atoms, only `CA`, or only side-chain atoms. All-atom gives the largest number of contacts; `CA`-only is the cheap approximation used in most coarse contact maps.
2. **The cutoff.** 8 Å is common for all-atom contacts; 5 Å is used for tighter definitions; 12 Å for a "residue neighbourhood". The number changes the map, so it belongs in the output.
3. **Whether |i - j| is excluded.** Neighbouring residues are always in contact, so the diagonal band is uninformative. Most maps omit it or mask it; a map that includes it looks like it has structure when it does not.

The early `return 1` inside the double loop is the whole of the cost model: contact detection stops at the first close pair, so the average cost is far below the worst case.

## 2. The naive cost

For `1A3N`, about 1400 residues:

\[
1400^2 \approx 2 \times 10^6 \text{ residue pairs}
\]

and with about 8 atoms per residue per pair that is \(1.5 \times 10^7\) distance computations. On a modern machine that is a few milliseconds — fast enough that the naive version is the right answer for this size. The scaling is what makes it wrong at 10× the residues: \(14\,000^2\) is 100× the pairs.

| Residues | Residue pairs | Estimated time (all-atom, 8 Å) |
| --- | --- | --- |
| 46 (`1CRN`) | 2 116 | under 1 ms |
| 76 (`1UBQ`) | 5 776 | under 1 ms |
| 1 400 (`1A3N`) | 1.96 M | a few ms |
| 20 000 | 400 M | several seconds |
| 100 000 | 10 000 M | minutes to hours |

The break-even for the naive version is around 10⁴ residues; beyond that the two improvements below are not optional.

## 3. Improvement one: spatial binning

Divide the bounding box into cells of side `cutoff`, so that any pair within the cutoff must be in the same cell or an adjacent one. Then each residue compares against a fixed number of neighbours rather than against all n.

```c
/* cells of side CUTOFF; a residue only touches its own and 26 neighbours */
size_t cells_per_side = (size_t)ceil(box_side / CUTOFF);
/* build the cell index once, then query 27 cells per residue */
```

The cost is \(O(n)\) for the index plus \(O(n \cdot k)\) for the queries, where k is the average number of residues in 27 cells — a constant determined by density, not by n. For a uniformly filled box, k is roughly 27× the number of residues in one cell, which stays small as n grows because the box grows with the molecule's extent.

The catch is that a protein is not uniformly dense: the surface is denser than the core in terms of neighbours within 8 Å, and a structure like a coiled-coil has regions where the local density is much higher. The constant is empirical, and the lab measures it rather than assuming.

## 4. Improvement two: sorted coordinate sweep

Sort the atoms along one axis, then for each atom scan forward until the x-difference exceeds the cutoff. This is the one-dimensional version of binning and is far simpler:

```c
qsort(atoms, n, sizeof *atoms, by_x);
for (size_t i = 0; i < n; ++i) {
    for (size_t j = i + 1; j < n && atoms[j].x - atoms[i].x < CUTOFF; ++j) {
        if (dist2(&atoms[i], &atoms[j]) < CUTOFF2) { mark_pair(i_orig, j_orig); }
    }
}
```

The inner loop terminates on the x-test, which is one comparison, and the full distance is computed only for the survivors. The break condition relies on the sort, so the sort is part of the algorithm rather than an optimisation — and the original indices must be carried through, because the map is indexed by residue identity, not by sorted position.

## 5. Storing the map

A `bool` array of n² costs n² bytes: 9 MB for 3000 atoms, 400 MB for 20 000. A bit array costs n²/8:

```c
/* 3000 residues: 1.1 MB as bits, 9 MB as bytes */
uint64_t *map = calloc((n * n + 63) / 64, sizeof *map);
map[i * n + j]  ->  bit index i*n+j, word = idx/64, bit = idx%64
```

The bit operations are the ones from chapter 10, applied to a real data structure. The saving matters for a map that is written to disk or compared against another: 1.1 MB versus 9 MB is the difference between a file that loads instantly and one that does not.

For symmetric maps the upper triangle is the whole information, so half the bits are wasted. A packed upper-triangular index is an exercise; the lab keeps the square layout because the indexing is simpler and the saving is a factor of two.

## 6. Field note: the map whose diagonal was all contacts

A first version of the program produced a contact map that was solid along the diagonal and mostly empty elsewhere, and the reported number of contacts was about twice the expected value.

```c
for (i) for (j) if (dist2(&a[i], &a[j]) < CUTOFF2) { map[i][j] = 1; }
```

Two defects in one line. The diagonal is trivially true — a residue is at distance 0 from itself — so the map claimed every residue was in contact with itself. And the loop counted each pair twice, because `(i, j)` and `(j, i)` are both visited, so any statistic over the map was doubled.

The repairs are `j = i + 1` in the inner loop and an explicit mask for the diagonal band. The generalisation: a symmetric quantity computed by a full double loop needs `j > i` and a stated convention for the diagonal, and a map that does not exclude `|i - j| <= 2` is reporting the polymer backbone rather than tertiary contacts.

## 7. Practise

Build `code/bio/07_contacts/contact_map.c` and run `bash labs/bio/07_contacts/check.sh`: the program builds a map from an embedded fragment, asserts that the binned version reproduces the naive one exactly, and prints both times. Downloading `1CRN` and repeating at 8 Å — where the upper-triangle contact count should be in the low hundreds — is the extension; a binning implemented as a re-scan shows up there as a time ratio near 1.

## Continue

- [Geometry](geometry.md) is the distance arithmetic underneath.
- [Parallel Scoring](parallel-scoring.md) distributes the same loop across threads.
- [Bit-Level Programming](../c/bit-level-programming.md) is the bit-array storage.
