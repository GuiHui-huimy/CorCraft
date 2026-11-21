# Geometry

Geometry is where a biology student meets floating point whether they want to or not. I start with distance and the comparison that avoids a square root, then dihedrals with their sign convention, then RMSD and why superposition must come first, then radius of gyration, then the angle between three atoms. The field note is a NaN that came out of a cosine slightly above one.

Four numbers come out of a structure file and everything else is arithmetic on them: distance, angle, dihedral, and radius of gyration. Each is a small formula with one non-obvious numerical trap, and this page is about those traps.

## 1. Distance, and the comparison without a square root

```c
/* LAB 06-01  bio_geometry/dihedral.c */
static double dist2(const atom *a, const atom *b) {
    double dx = a->x - b->x, dy = a->y - b->y, dz = a->z - b->z;
    return dx * dx + dy * dy + dz * dz;
}

static double dist(const atom *a, const atom *b) { return sqrt(dist2(a, b)); }
```

The squared form exists because most distance questions are comparisons: "is this pair closer than 8 Å?" is `dist2 < 64.0`, and comparing squared values avoids a square root per pair. In a contact search over \(3000^2\) pairs that is 4.5 million square roots saved, which is a measurable fraction of the loop's time.

The trap is a threshold that was computed with a square root and compared with a squared value. The units are the only warning, and a comment stating "squared" on the function is the cheapest defence.

```c
if (dist(a, b) < cutoff)          // 4.5M sqrt calls
if (dist2(a, b) < cutoff * cutoff) // same answer, no sqrt
```

## 2. Dihedral angles

A dihedral is the angle between the planes defined by atoms 1–2–3 and 2–3–4, measured about the 2–3 axis. The standard formulation:

\[
b_1 = r_2 - r_1,\quad b_2 = r_3 - r_2,\quad b_3 = r_4 - r_3
\]
\[
n_1 = b_1 \times b_2,\quad n_2 = b_2 \times b_3
\]
\[
\phi = \operatorname{atan2}\left( (n_1 \times n_2) \cdot \frac{b_2}{|b_2|},\ n_1 \cdot n_2 \right)
\]

```c
static double dihedral(const atom *a, const atom *b, const atom *c, const atom *d) {
    vec b1 = sub(b, a), b2 = sub(c, b), b3 = sub(d, c);
    vec n1 = cross(b1, b2), n2 = cross(b2, b3);
    double y = dot(cross(n1, n2), unit(b2));   /* signed */
    double x = dot(n1, n2);
    return atan2(y, x) * 180.0 / M_PI;         /* degrees, (-180, 180] */
}
```

`atan2` rather than `acos`: the cosine alone cannot distinguish \(+\phi\) from \(-\phi\), and a peptide bond's sign is the difference between a cis and a trans conformation. The `atan2` form returns the full range and needs no clamping.

The sign convention is a source of disagreement between programs: some define \(\phi\) with the opposite sense, so a comparison against literature values requires knowing which convention produced them. The labs fix the convention in the header comment and do not compare dihedrals against external sources.

## 3. RMSD, and why superposition comes first

The naive RMSD between two conformations:

```c
double rmsd = 0.0;
for (size_t i = 0; i < n; ++i) { rmsd += dist2(&a[i], &b[i]); }
rmsd = sqrt(rmsd / n);
```

This is the correct formula and usually the wrong number, because it depends on the coordinate frame. Two identical structures translated by 10 Å have an RMSD of 10 Å; rotated by 90°, an RMSD of up to the molecular diameter. What is wanted is the RMSD after the optimal rigid superposition — the Kabsch algorithm — which is an SVD of a 3×3 covariance matrix.

The chapter states the result and points at the lab rather than deriving Kabsch, because the derivation is a linear algebra chapter and the point here is the distinction:

```c
/* after superposition */
rmsd_superposed <= rmsd_raw
```

with equality only when the structures are already aligned. Reporting a raw RMSD as a structural difference is a defect that reviewers catch, and the reference program checks both numbers for that reason.

The lab implements the first half: `rmsd.c` aligns the centroids and then shows, on a rotated copy, why translation alone cannot fix a rotation. The full fit multiplies a 3×3 covariance matrix by a rotation taken from an SVD; that is six pages of linear algebra, and the exercise is to build it correctly rather than to derive it.

## 4. Radius of gyration

\[
R_g^2 = \frac{1}{N} \sum_i |r_i - r_{\text{cm}}|^2
\]

```c
/* LAB 06-02  bio_geometry/rmsd.c */
vec cm = {0, 0, 0};
for (size_t i = 0; i < n; ++i) { cm = add(cm, a[i].pos); }
cm = scale(cm, 1.0 / (double)n);

double sum = 0.0;
for (size_t i = 0; i < n; ++i) { sum += dist2(&a[i], &cm_atom); }
double rg = sqrt(sum / n);
```

Two passes: one for the centroid, one for the sum. The centroid must be computed before the distances, which is why the formula is written as a squared distance to a centroid rather than expanded — the expansion \(\sum |r_i|^2 - N|r_{\text{cm}}|^2\) is algebraically equal and numerically worse for large coordinates, because the two terms are large and their difference is small.

`1UBQ`'s \(R_g\) is about 11.7 Å, which is a number worth memorising as a sanity check for a parser: if the computed value is 117 or 1.17, the units or the indexing is wrong.

## 5. Angle between three atoms

\[
\cos\theta = \frac{(r_1 - r_2) \cdot (r_3 - r_2)}{|r_1 - r_2|\,|r_3 - r_2|}
\]

```c
double c = dot(u, v) / (norm(u) * norm(v));
if (c > 1.0) { c = 1.0; } else if (c < -1.0) { c = -1.0; }
double theta = acos(c);
```

The clamp is not optional. Computed in floating point, the cosine of a nearly straight angle can come out as `1.0000000000000002`, and `acos` of that is NaN — not an error, a quiet NaN that propagates through every subsequent sum and turns the final answer into `nan`. The reference program asserts that every angle it prints is finite, which is the only reliable detector because a NaN does not compare equal to anything.

The alternative that avoids the clamp entirely is `atan2(|u × v|, u · v)`, which is well-conditioned for all angles and needs no branch. Both are correct; the `acos` form is what most code uses, and the clamp is what most code forgets.

## 6. Field note: the nan that came from a cosine

A course program computed bond angles and printed a table where a few entries were `-nan`. The cause was the missing clamp, and the trigger was a nearly linear arrangement of three atoms — a real geometry in a real structure, not a contrived one.

The pattern is worth recognising: `acos`, `asin`, `sqrt` of a negative, and division by a norm of zero all produce NaN or inf, and the value then spreads. A single NaN in a scoring sum turns the whole score into NaN, and a comparison like `score < threshold` against NaN is false, so the record is silently dropped rather than flagged.

The repair in this book is two-part: clamp at the source, and refuse to propagate — the parsing code and the scoring code both check `isfinite` at their boundaries and abort with the input that produced the bad value.

## 7. Practise

Build `code/bio/06_geometry/dihedral.c` and `rmsd.c`, then run `bash labs/bio/06_geometry/check.sh`. Both work on embedded coordinates: the torsion angles are checked against hand-computed values of 180° and 90°, and the RMSD is checked twice — it falls to zero on a translated copy after centroid alignment, and it stays non-zero on a rotated one, which is what shows that alignment removes translation and not rotation. The full Kabsch fit, which needs an SVD, is the next step. Computing `1UBQ`'s \(R_g\) and comparing it with 11.7 Å is the extension, once the file is on disk.

## Continue

- [Contacts](contacts.md) uses the squared-distance comparison at scale.
- [Numerics for Scoring](numerics.md) is the floating-point background for the clamps.
- [PDB Format](pdb-format.md) is where the coordinates come from.
