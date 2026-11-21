# Sequence Encoding

Counting is the first real algorithm in this part, and I use it to introduce the idea that an encoding is a decision about memory. Plain characters, then composition, then k-mers, then 2-bit packing with its four-symbol limit, then a table that puts the three side by side. The bug at the end is an index that went negative, which is what happens when you trust an unsigned length.

Once a sequence is in memory as characters, every question about it is a counting question, and the cost of a counting question depends almost entirely on how the characters are laid out. This page compares three encodings that trade memory against speed against clarity.

## 1. Counting, the simple version

```c
/* LAB 02-01  bio_encoding/gc_kmer.c */
size_t counts[26] = {0};
for (size_t i = 0; i < n; ++i) {
    int idx = to_index(seq[i]);          /* 0..25 */
    if (idx >= 0) { ++counts[idx]; }
}
```

One array access per residue, and the array is 208 bytes — it stays in L1 for the whole loop. This is the right answer for residue composition, and it is fast enough that the encoding question does not arise.

```c
static int to_index(char c) {
    if (c >= 'A' && c <= 'Z') { return c - 'A'; }
    if (c >= 'a' && c <= 'z') { return c - 'a'; }
    return -1;
}
```

The `-1` is the whole of the validation. A parser that normalises case before this point makes the second branch unnecessary, and one that does not needs it. The defect this guards against is `idx = c - 'A'` computed on a lowercase character, which yields 32 for `'a'` and writes eight elements past the end of a 26-element array — a stack buffer overflow that ASan catches immediately and that no amount of correct arithmetic hides.

## 2. Composition and GC fraction

```c
size_t gc = counts['G' - 'A'] + counts['C' - 'A'];
size_t at = 0;
for (int i = 0; i < 20; ++i) { at += counts[residue_letter(i) - 'A']; }
double fraction = (double)gc / (double)at;
```

Two decisions in three lines.

The denominator is the count of *residues*, not the length of the sequence. A sequence containing `*` or an ambiguous code (`X`, `B`, `Z`) has a length larger than its residue count, and dividing by the length reports a GC fraction that is too small. The difference is small per sequence and systematic across a genome.

The numerator counts `G` and `C` only. Whether `S` (G or C) counts as GC is a convention question, and the convention must be stated in the code's comment rather than left to the reader.

`(double)gc / (double)at` casts both operands. Casting only the numerator also works, because the division promotes, but casting both makes the intent visible and prevents the integer-division defect from reappearing when someone edits the line.

## 3. k-mer counting

A k-mer is a substring of length k. For k = 3 there are 4³ = 64 possible DNA 3-mers and 20³ = 8000 possible protein 3-mers.

```c
size_t kmer[64] = {0};        /* DNA, k = 3 */
for (size_t i = 0; i + 3 <= n; ++i) {
    int code = 16 * base_code(seq[i]) + 4 * base_code(seq[i + 1]) + base_code(seq[i + 2]);
    ++kmer[code];
}
```

The code is base-4 arithmetic: `A=0, C=1, G=2, T=3` gives a unique index for every 3-mer, computed with two shifts and two additions. This is the same masking idea as chapter 10's bit fields, expressed in base 4.

The table is 512 bytes and in L1, so each iteration is one load and one store. The alternative — building a `char[4]` key and hashing it — costs a hash computation, a bucket walk, and a cache miss per k-mer, and is an order of magnitude slower for no benefit.

## 4. 2-bit packing

A DNA sequence of length n needs 2n bits: four bases, two bits each. For a 3 Gbp genome that is 750 MB instead of 3 GB, and for a 1 Mbp gene it is 250 KB instead of 1 MB.

```c
/* LAB 02-02  bio_encoding/two_bit.c */
typedef struct {
    uint64_t *words;
    size_t n_bases;
} packed_seq;

static void set_base(packed_seq *s, size_t i, uint8_t code) {
    size_t w = i / 32, b = (i % 32) * 2;
    s->words[w] = (s->words[w] & ~(3ULL << b)) | ((uint64_t)code << b);
}

static uint8_t get_base(const packed_seq *s, size_t i) {
    size_t w = i / 32, b = (i % 32) * 2;
    return (uint8_t)((s->words[w] >> b) & 3ULL);
}
```

`~(3ULL << b)` clears the two bits before ORing the new value: read-modify-write, one shift, one mask, one or. Missing the clear turns `set` into an OR that can only add bits, which is the defect this lab is written to catch — set a base to `G` after `A` and read it back as `G`, then set it back to `A` and read `G` again.

The packing is worth it only when the sequence is large or is stored many times. For a single 76-residue protein it costs more in shifts than it saves in memory. The chapter's claim is not that packing is always better; it is that the trade is arithmetic and should be made deliberately.

## 5. The three encodings side by side

| Encoding | Memory per residue | Access cost | Where it wins |
| --- | --- | --- | --- |
| `char` | 1 byte | one load | parsing, everything except storage |
| `char` + count table | 1 byte + 208 B | one load, one increment | composition, k-mers |
| 2-bit packed | 0.25 byte | two shifts, mask, load | storage, transmission, cache |

The count table is not an encoding of the sequence; it is a summary. Both are needed, for different questions, and a program that keeps only the summary cannot answer a question about position.

## 6. Field note: the index that went negative

The archived `absCal.c` computed an absolute value as:

```c
int absF(int x) {
    if (x < 0) { return -x; }
    return x - 1;
}
```

For a non-negative input the function subtracts one. The function is named for absolute value and implements `|x|` for negatives and `x - 1` for non-negatives — a branch that was probably a `return x` with a stray edit, and which no test caught because the only caller passed negative numbers.

The parallel in this chapter is the index computation: both are a one-character deviation in a branch that looks plausible. The defence is the same in both cases — a test with inputs on both sides of the branch, and a name that states the contract. `absF` promises `|x|`; the test that would have caught it is `assert(absF(5) == 5)`.

## 7. Practise

Build `code/bio/02_encoding/gc_kmer.c` and verify the GC fraction of ubiquitin against a hand count. Then attempt the exercise in `labs/bio/02_encoding/`: `two_bit.c` carries a position-by-position round-trip assertion over a packed sequence, plus an explicit word-boundary check at positions 31 and 32, and the project flags build it under ASan so that any off-by-one word index is reported rather than tolerated.

## Continue

- [FASTA and Sequence Input](fasta.md) produces the characters this page encodes.
- [Alignment](alignment.md) scores sequences against each other.
- [Bit-Level Programming](../c/bit-level-programming.md) is the C-side of the packing arithmetic.
