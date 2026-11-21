# Alignment

Alignment is the one algorithm in this part that every reader has used and few have implemented. I derive the recurrence, then show that the table is the cost, then do linear space without traceback, then the scoring numbers and where they come from, then tie-breaking, because a different traceback path on equal scores is a reproducibility bug rather than a detail.

Alignment is the oldest algorithm in this book and the one that most clearly shows the cost of a table. Two sequences of length n and m, a scoring rule, and a decision at each cell: match, insert, or delete. The naive implementation needs n×m cells; the interesting part is how little of that table is actually needed.

## 1. The recurrence

Global alignment of sequences \(a\) and \(b\) with a linear gap penalty:

\[
M(i,j) = \max \begin{cases} M(i-1,j-1) + s(a_i, b_j) \\ M(i-1,j) - g \\ M(i,j-1) - g \end{cases}
\]

with \(M(0,0) = 0\), \(M(i,0) = -ig\), \(M(0,j) = -jg\). The final score is \(M(n,m)\), and the alignment is recovered by tracing back from \((n,m)\) to \((0,0)\).

```c
/* LAB 04-01  bio_alignment/nw.c */
int *score = malloc((n + 1) * (m + 1) * sizeof *score);
int *trace = malloc((n + 1) * (m + 1) * sizeof *trace);   /* 0 diag, 1 up, 2 left */

for (size_t i = 1; i <= n; ++i) {
    for (size_t j = 1; j <= m; ++j) {
        int diag = score[(i - 1) * (m + 1) + (j - 1)] + sub(a[i - 1], b[j - 1]);
        int up   = score[(i - 1) * (m + 1) + j] - gap;
        int left = score[i * (m + 1) + (j - 1)] - gap;
        int best = diag;
        int dir = 0;
        if (up > best)   { best = up;   dir = 1; }
        if (left > best) { best = left; dir = 2; }
        score[i * (m + 1) + j] = best;
        trace[i * (m + 1) + j] = dir;     /* only needed for traceback */
    }
}
```

The table is `(n+1)*(m+1)` ints, and the traceback needs one direction per cell. For two 1000-residue proteins that is 4 MB of scores and 4 MB of directions, which fits in memory and misses the cache on nearly every cell for the larger sizes.

## 2. The cost is the table, not the arithmetic

| Sequences | Cells | Score table | With directions |
| --- | --- | --- | --- |
| 76 × 76 | 5 929 | 24 KB | 48 KB |
| 300 × 300 | 90 601 | 362 KB | 725 KB |
| 1000 × 1000 | 1 002 001 | 4 MB | 8 MB |
| 10 000 × 10 000 | 100 020 001 | 400 MB | 800 MB |

The 10 000 × 10 000 case is where the textbook implementation stops being usable: 400 MB of table for a comparison that needs two rows at a time. This is the moment the chapter's memory theme pays off — the algorithm is O(nm) time and can be O(min(n,m)) space, and the difference is entirely in how the table is stored.

## 3. Linear space without traceback

```c
int *row[2];                  /* two rows, swapped */
for (size_t i = 1; i <= n; ++i) {
    for (size_t j = 1; j <= m; ++j) {
        /* reads from row[(i-1)%2], writes to row[i%2] */
    }
}
```

Only the previous row is needed to compute the current row, so two rows suffice: 8 KB for a 1000×1000 alignment instead of 4 MB. The catch is that the traceback needs the whole table or a recomputation. Hirschberg's algorithm solves that with divide and conquer and two linear passes per level, giving O(nm) time and O(min(n,m)) space with the alignment recovered. It is an exercise, not a chapter.

The practical rule for this book: store directions only when the alignment itself is the output. A score-only call — "how similar are these two" — uses two rows and writes no directions.

## 4. Scoring, and where the numbers come from

```c
static int sub(char x, char y) { return x == y ? 1 : -1; }
```

This is the simplest substitution score: identity. Real alignment uses a matrix such as BLOSUM62, where conservative substitutions (I→V, D→E) score positive and radical ones negative. The matrix is a 20×20 table of `int8`, 400 bytes, indexed by the same residue index as the encoding chapter:

```c
static const int8_t blosum62[20][20] = { ... };
static int sub(char x, char y) { return blosum62[idx(x)][idx(y)]; }
```

The reason to include one real matrix in the lab: it makes the score magnitudes realistic, and it exposes the difference between a score that is a count (identity) and a score that is a log-odds (BLOSUM). The two produce alignments of different character on the same input, and the lab prints both.

The log-odds part is the piece I had to read a paper to understand, so I state it plainly here. BLOSUM62's entry for a pair of residues is \(2\log_2\frac{p_{\text{observed}}}{p_{\text{expected}}}\): the ratio of how often the pair occurs in the curated blocks versus how often it would occur if residues paired independently, in half-bit units, rounded to an integer. Two consequences follow. First, the scores add, because a product of ratios becomes a sum of logarithms — that is the only reason a per-position score can be summed along an alignment at all. Second, the score has no absolute meaning: it is always relative to the background model, which is why the same matrix used with different gap costs produces a different alignment, and why a negative entry means "less frequent than random", not "chemically impossible".

Gap penalties come in two forms. A **linear** penalty charges `g` per gap character; an **affine** penalty charges `open + extend * length`, which is the correct model because a single indel of length 5 is one evolutionary event, not five independent ones. Affine penalties need three states per cell (in-match, in-gap-a, in-gap-b) rather than one, which triples the table. That cost is the reason the affine version is a second lab rather than the default.

## 5. Reproducibility of ties

Two cells can have equal scores, and the direction chosen for a tie determines the alignment reported. Different implementations break ties differently, so two programs computing the same score can return different alignments. The book's labs fix the order explicitly — diagonal preferred, then up, then left — and state it in the header comment, because an task that compares alignments byte-for-byte needs the tie-break rule to be part of the contract.

## 6. Field note: the off-by-one that shifted a whole alignment

A course implementation used 0-based indexing throughout but allocated a table with an extra row and column for the boundary:

```c
for (i = 1; i <= n; i++)
    for (j = 1; j <= m; j++)
        score[i][j] = score[i-1][j-1] + sub(a[i], b[j]);   /* a[i] is the NEXT residue */
```

`a[i]` with a 1-based loop reads past the last residue on the final iteration and compares the wrong characters on every iteration before it. The reported score was plausible — the sequences were similar — and the alignment was shifted by one position throughout.

The defect is a naming problem before it is an arithmetic problem: the code mixed a 1-based loop with a 0-based array without ever writing down which convention applied where. The repair used in this book is to keep the sequence 0-based and the table 1-based, and to name the accessor so the shift is visible:

```c
static int cell(const int *t, size_t i, size_t j, size_t m) { return t[i * (m + 1) + j]; }
```

## 7. Practise

Build `code/bio/04_alignment/nw.c` and align ubiquitin against itself — the score must be the identity score and the alignment must be trivial. Then attempt the exercise in `labs/bio/04_alignment/`, which requires a correct traceback for two sequences with a known single-residue deletion, and a second run that recomputes the score with two rows only and must match the full-table version exactly.

## Continue

- [Numerics for Scoring](numerics.md) explains why the score sums are accumulated carefully.
- [Parallel Scoring](parallel-scoring.md) is the same loop with the outer dimension distributed.
- [Arrays and Layout](../c/arrays-and-layout.md) is the indexing arithmetic this page depends on.
