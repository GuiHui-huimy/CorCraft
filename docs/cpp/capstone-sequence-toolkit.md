# Capstone: Sequence Toolkit

The capstone is where the three tracks meet, so I state the contract before the code: a sequence type that owns its storage, a scoring function that returns a fraction, and a report that prints without allocating per line. Then I build the layers in that order, and each section ends with the check the program performs on itself. If the earlier chapters worked, this page is assembly rather than instruction; if one of them did not, the failing assertion tells you which.

The capstone assembles the C++ half of the book into one program: a FASTA reader, a composition calculator, and a report writer, with an owned buffer instead of a raw pointer, a template where the type genuinely varies, and an operator overload where the type genuinely has an invariant.

Nothing here is new. The point is that the pieces compose, and that each defect class from the previous twelve chapters has a place where it would show up if the piece were written carelessly.

## 1. The three layers

```text
Sequence          owns name and residues, reads FASTA, validates the alphabet
Composition       counts residues, computes GC fraction as a Fraction
Report            formats rows, writes to stdout or a file
```

The layers are separated by ownership. `Sequence` owns its characters and never lends a pointer out. `Composition` borrows a `const Sequence &`. `Report` borrows a `Composition` and owns nothing. No layer can invalidate another layer's data, which is the structural version of the lifetime rules from chapter 3.

## 2. Layer 1: `Sequence` owns

```cpp
// LAB 13-01  cpp_capstone/sequence_toolkit.cpp
class Sequence {
public:
    static std::optional<Sequence> parse(std::string_view header, std::string_view body) {
        if (header.empty() || !header.starts_with('>')) { return std::nullopt; }
        Sequence s;
        s.name_ = header.substr(1);
        s.residues_.reserve(body.size());          // known size: one allocation
        for (char c : body) {
            if (c == '\n' || c == '\r' || c == ' ') { continue; }
            if (!is_residue(c)) { return std::nullopt; }
            s.residues_.push_back(c);
        }
        return s;
    }
    std::size_t size() const { return residues_.size(); }
    char operator[](std::size_t i) const { return residues_[i]; }
    const std::string &name() const { return name_; }
private:
    static bool is_residue(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '*';
    }
    std::string name_;
    std::vector<char> residues_;     // owns: no raw pointer, no manual free
};
```

`std::optional<Sequence>` rather than a partially valid object: a sequence either parsed completely or does not exist. This is chapter 11's rule applied at the boundary — invalid input produces no object rather than an object with a bad flag.

`reserve` before the loop is chapter 9's rule; the size is known from the body, so the one allocation is the only allocation.

## 3. Layer 2: `Composition`, and `Fraction`

```cpp
class Fraction {
public:
    Fraction(long num, long den) : num_(num), den_(den) {
        if (den == 0) { throw std::invalid_argument("zero denominator"); }
        long g = std::gcd(num, den);
        num_ /= g; den_ /= g;
        if (den_ < 0) { num_ = -num_; den_ = -den_; }
    }
    long num() const { return num_; }
    long den() const { return den_; }
    friend Fraction operator*(const Fraction &a, const Fraction &b) {
        return Fraction(a.num_ * b.num_, a.den_ * b.den_);   // constructor reduces
    }
    friend bool operator==(const Fraction &a, const Fraction &b) {
        return a.num_ == b.num_ && a.den_ == b.den_;
    }
private:
    long num_, den_;
};
```

The invariant — denominator positive, fraction reduced, denominator nonzero — is established in the constructor and preserved by every operator that returns a `Fraction`, because every such operator goes through the constructor. This is chapter 6's argument, in the one place the capstone needs a number that is exactly representable: a GC fraction is `71/100`, not `0.71`, and the report prints it reduced.

```cpp
class Composition {
public:
    explicit Composition(const Sequence &s) : counts_{} { for (...) ++counts_[index(c)]; }
    Fraction gc_fraction() const {
        std::size_t gc = 0, at = 0;
        for (char c : {'G', 'C', 'c', 'g'}) { gc += counts_[index(c)]; }
        for (std::size_t i = 0; i < 26; ++i) { at += counts_[i]; }
        return Fraction(long(gc), long(at));       // reduces in the constructor
    }
    std::size_t count(char c) const { return counts_[index(c)]; }
private:
    std::array<std::size_t, 26> counts_;
};
```

`std::array` with a fixed size and no allocation is chapter 7's `FixedBuffer` idea, and the index computation is chapter 10's masking, scaled down to a lookup.

## 4. Layer 3: `Report` writes, and owns nothing

```cpp
void write_report(std::ostream &out, const std::string &name, const Composition &comp) {
    out << name << '\t' << comp.gc_fraction().num() << '/'
        << comp.gc_fraction().den() << '\n';
}
```

Taking `std::ostream &` rather than writing to `std::cout` is what makes the output comparable: the program assembles the report in a `std::ostringstream`, asserts on its prefix, and only then prints it. The same argument is why a harness could freeze this output later without touching the code.

## 5. What the program checks

`labs/cpp/13_capstone/check.sh` builds the target with the project flags and runs it; the assertions inside `code/cpp/13_capstone/sequence_toolkit.cpp` are what decide the result:

1. Ubiquitin (P0CG48) parses to 76 residues and keeps its header text.
2. The report's leading fields — name, count, reduced GC fraction — match the expected prefix, compared with `rfind`.
3. `Fraction` reduces `6/8` to `3/4`, normalises `1/-2`, maps `0/5` to `0/1`, and throws on a zero denominator.
4. Three malformed records are rejected rather than sanitised: a header without `>`, a body with a digit, an empty record.
5. Lowercase input is accepted and counted like uppercase.
6. The run is clean under AddressSanitizer and UndefinedBehaviorSanitizer, and the script prints any warning the build emitted.

The sanitizer is the part an output comparison cannot do: a fraction that prints correctly through a dangling reference would still match the string. That is why the harness builds with the sanitizers rather than only running the binary.

## 6. Why this capstone and not a bigger one

The sequence toolkit is small enough to hold in one's head and large enough to contain every chapter's failure mode: an owned buffer (chapter 8), a template (`std::array`, chapter 7), an operator with an invariant (chapter 6), an optional boundary (chapter 11), a move-only type (`Sequence` is copyable but the vector inside makes it a real candidate for moving, chapter 3), and an exception at a boundary (chapter 12).

The biology section builds on exactly this structure: the FASTA reader there is the same `parse` function with more validation, and the PDB parser is the same ownership discipline applied to a hierarchical format. The capstone is the bridge, which is why it is the last C++ page.

## 7. Practise

`labs/cpp/13_capstone/` holds only `check.sh`. Copy `sequence_toolkit.cpp` into `/tmp`, delete the bodies of `Sequence::parse`, `Composition::gc_fraction` and `write_report`, and rebuild them from this chapter with the signatures unchanged. Then add one test the script does not cover — a case you think of, not one given to you — and run your binary with the same sanitizer flags the project uses.

## Continue

- [FASTA and Sequence Input](../bio/fasta.md) is the same parser with real-world edge cases.
- [PDB Format](../bio/pdb-format.md) is the ownership discipline applied to a hierarchical file.
- [Testing and Sanitizers](../engineering/testing.md) explains what `check.sh` is doing.
