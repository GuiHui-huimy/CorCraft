# Glossary

Definitions only, grouped by the section that uses them first: memory, C and C++, build, numbers, biology. I keep them short, because a glossary that argues is a chapter in disguise.

Terms as this book uses them, which is not always how the standard defines them. Where the two differ, the book's usage is what the chapters rely on.

## 1. Memory

**Object** — a region of storage with a type. A variable is a named object; an expression's result is usually an unnamed one.

**Storage duration** — how long the storage exists: automatic (a scope), static (the program), dynamic (until freed), thread (the thread).

**Alignment** — the requirement that an object's address be a multiple of its `alignof`. A misaligned access is undefined behaviour and, on x86, a performance penalty.

**Padding** — bytes inserted between members to satisfy alignment. Never readable through a member, always present in `sizeof`.

**Undefined behaviour (UB)** — the standard imposes no requirement on the result. Not "a random value": the compiler may assume it never happens and delete the branch that checks for it.

**Unspecified behaviour** — a set of allowed results; the implementation picks one and does not have to document which.

**Implementation-defined** — a documented choice, such as the width of `int`.

**Strict aliasing** — the rule that two pointers of incompatible types do not point at the same object. Violating it is UB, and `-fno-strict-aliasing` relaxes it.

**Effective type** — the type of the last store to an allocated object; the aliasing rule is stated in terms of it.

**Arena / region** — a block of memory from which many small objects are carved, with one free for the whole block. The alternative to one `malloc` per object.

## 2. C and C++

**Translation unit (TU)** — one source file after preprocessing, including every header it pulled in. The compiler sees only TUs.

**ODR** — the one-definition rule: a function or object with external linkage is defined once across all TUs. Class definitions may repeat, but must be token-identical.

**Internal linkage** — a name visible only within its TU: `static` at file scope, or an anonymous namespace in C++.

**Lvalue / rvalue** — a value with an identity (address) versus a value without one. C++'s value categories refine this into lvalue, xvalue and prvalue.

**RAII** — resource acquisition is initialisation: an object acquires a resource in its constructor and releases it in its destructor, so every exit path releases it.

**Special member functions** — default constructor, copy, move, destructor. Declaring one suppresses others; this is the rule of five.

**Rule of zero** — the recommendation that a class declare none of the five, because its members are RAII types that already do it correctly.

**Slicing** — copying a derived object into a base object, losing the derived part. Happens when a polymorphic object is passed by value.

**Virtual dispatch** — the call through a vtable that reaches the most-derived override. Two indirections: object to vtable pointer, vtable to function pointer.

**Template instantiation** — generating a concrete function or class from a template and its arguments. Lazy: a member function is instantiated only when used.

**Two-phase lookup** — names in a template are resolved at definition (non-dependent) and at instantiation (dependent), which is why a declaration order can break a template.

**Concept** (C++20) — a named set of constraints on template parameters, checked at the call site with a readable diagnostic.

## 3. Build

**Object file** — the output of assembling one TU: code, data, and a symbol table. Not yet linked.

**Symbol** — a name in an object file, marked defined (`T`) or undefined (`U`).

**Linkage** — how a symbol is resolved across TUs: external, internal, or none.

**Static library** — an archive of object files, copied into the executable at link time. `.a` on Linux, `.lib` on Windows.

**Shared library** — loaded at run time, shared between processes. `.so` on Linux, `.dll` on Windows.

**Sanitizer** — a compiler instrumented runtime check. ASan wraps memory, UBSan checks operations, LSan scans at exit, TSan checks data races.

**Golden file** — expected output stored in the repository and compared byte-for-byte. The cheapest regression test and the reason output formatting must be platform-independent.

## 4. Numbers

**ULP** — unit in the last place: the gap between a floating-point value and the next representable one. Error bounds are stated in ULPs.

**Kahan summation** — compensated summation that carries the rounding error in a second variable, reducing the error from \(O(n\epsilon)\) to \(O(\epsilon)\).

**Log-sum-exp** — the numerically stable form of \(\log \sum e^{x_i}\), factoring out the maximum to avoid overflow and underflow.

**Amortised cost** — the average over a sequence of operations, used for `push_back` where a single call may be O(n) but the average is O(1).

**NaN** — a floating-point value that is not a number, produced by `0/0`, `acos` of an out-of-range argument, or an invalid operation. It propagates through arithmetic and compares false with everything, including itself.

**Strict weak ordering** — the requirement on a comparator: irreflexive, transitive, and with transitive incomparability. `<=` is not one, which is why a comparator must use `<`.

## 5. Biology

**Residue** — one amino acid in a chain, identified by chain, sequence number and insertion code.

**Altloc** — alternate location indicator: a structure may have two conformations of the same atom, labelled `A`, `B` in column 17.

**Backbone** — the `N`, `CA`, `C`, `O` atoms of a residue, which define the chain's path.

**Dihedral** — the angle between two planes defined by four consecutive atoms, measured about the middle bond. A peptide's \(\phi\) and \(\psi\) are dihedrals.

**RMSD** — root mean square deviation, the square root of the mean squared distance between corresponding atoms. Meaningful only after optimal superposition.

**Kabsch algorithm** — the SVD-based computation of the optimal rotation between two point sets. What makes an RMSD a structural comparison.

**Radius of gyration** — the root mean square distance of atoms from the centroid, a measure of compactness.

**Contact map** — an n×n matrix marking residue pairs within a cutoff distance. The simplest tertiary-structure descriptor.

**k-mer** — a substring of length k, used for composition and feature encoding.

**PDB format** — the Protein Data Bank's fixed-column text format. Every field is defined by columns, not by delimiters.

**FASTA format** — a header line starting with `>` followed by residues. The format specifies no line length, case, or uniqueness.

## Continue

- [C Reference](c.md) and [C++ Reference](cpp.md) are the tables these definitions support.
- [Repository Layout](repo-layout.md) is where the terms appear as directory names.
- [Overview: From Sequence to Structure](../bio/overview.md) defines the biology terms in context.
