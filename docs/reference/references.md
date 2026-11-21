# References

This is a course, not a paper, so the list is short and deliberately narrow: only the work whose results the chapters actually use. The derivations, the code and the chapter numbering are this project's own; where a number in the text came from a run, the lab that produced it is named in the text.

## 1. Sequence algorithms

- **Needleman, S. B. & Wunsch, C. D. (1970).** A general method applicable to the search for similarities in the amino acid sequence of two proteins. *Journal of Molecular Biology* 48(3), 443–453. — the recurrence and the traceback in the alignment chapter.
- **Hirschberg, D. S. (1975).** A linear space algorithm for computing maximal common subsequences. *Communications of the ACM* 18(6), 341–343. — the O(min(n, m)) traceback that the alignment chapter leaves as an exercise.
- **Henikoff, S. & Henikoff, J. G. (1992).** Amino acid substitution matrices from protein blocks. *Proceedings of the National Academy of Sciences* 89(22), 10915–10919. — BLOSUM62 and the half-bit log-odds scale used for scoring.

## 2. Numerical methods

- **Kahan, W. (1965).** Further remarks on reducing truncation errors. *Communications of the ACM* 8(1), 40. — compensated summation in the numerics chapter.
- **Goldberg, D. (1991).** What every computer scientist should know about floating-point arithmetic. *ACM Computing Surveys* 23(1), 5–48. — background for the floating-point behaviour the C chapters treat as undefined or unspecified.

## 3. Bit-level programming

- **Kernighan, B. W. & Ritchie, D. M. (1988).** *The C Programming Language*, 2nd edition. Prentice Hall. — the `v &= v - 1` loop, still the standard name for "clear the lowest set bit".

## 4. Parallelism and memory

- **Amdahl, G. M. (1967).** Validity of the single processor approach to achieving large scale computing capabilities. *AFIPS Conference Proceedings* 30, 483–485. — the bound behind every "how much faster can this get" argument in the performance chapter.
- **Drepper, U. (2007).** *What Every Programmer Should Know About Memory.* Red Hat. — cache lines, prefetchers and the reason a stride costs what it costs.

## 5. Standards

- **ISO/IEC 9899:2018** — C17. The final committee draft is free: [N2176](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2176.pdf). Clause numbers in this book follow it.
- **ISO/IEC 14882:2020** — C++20. The working draft is free: [N4861](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf).

## 6. File formats

- **wwPDB.** *Protein Data Bank Contents Guide: Atomic Coordinate Entry Format Description*, version 3.30. — the fixed columns and the record types parsed in the PDB chapter.
- **wwPDB.** *PDBx/mmCIF dictionary.* — the hierarchy of data blocks, categories and loops behind the structure model.

## 7. Deliberately not cited

Two things a reader may expect are absent on purpose.

- **BLAST and other aligners.** The book does not run an aligner; the algorithm is what chapter 04 teaches, and running someone else's binary is a shell exercise rather than a computing one. The original paper is not cited because no result in the book depends on it.
- **C and C++ textbooks.** Nothing in the text is taken from one, and a reading list of them would suggest otherwise. If you want one, the standard drafts above are the primary source and are more accurate than any summary of them.

If a citation here is wrong, or a chapter uses a result that is missing from this list, that is a defect worth reporting and will be fixed in the next version.
