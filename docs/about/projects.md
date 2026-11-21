# Projects

These are the four projects the book's examples come from, and I describe them in the order they taught me something: Protein Phaser for parsing, FlexTrimmer for sequence work, BioCUDA for parallelism, and the GROMACS scripts for the unglamorous business of reproducible analysis. The last section is what they have in common, which is the reason they are on this page at all.

Four projects come out of the same workflow this book describes. They are listed here because a reader who finishes the biology chapters will recognise their shapes: a parser, a geometry engine, a numerics layer, and a GPU port.

## 1. Protein Phaser

A C++ library for reading PDB and mmCIF files into a chain–residue–atom hierarchy, with geometric queries on top. Written in 2025.

| | |
| --- | --- |
| Language | C++20, no external dependencies |
| Input | PDB and mmCIF, fixed columns and the mmCIF loop format |
| Queries | pocket centre, residue depth, solvent-accessible surface area |
| Size | about 4000 lines |

What it shares with this book: the `Structure`/`Chain`/`Residue`/`Atom` model in [Structure Model](../bio/structure-model.md) is the same design, reduced to what a chapter can hold. The differences are scale — mmCIF support, the geometry engine, and the tests — and the reason the chapter is a chapter rather than a copy.

The lesson it produced: a parser that is correct on `1CRN` and wrong on `1A3N` is a parser whose tests were too easy. The multi-chain case is the one that finds the column error.

## 2. FlexTrimmer

A machine-learning tool that predicts truncation sites for a protein sequence, to improve expression yield. Feature engineering over sequence windows, a small model, and a ranked list of candidate cut points.

| | |
| --- | --- |
| Language | Python for the model, C++ for the feature extraction |
| Input | a sequence, a window size |
| Output | ranked truncation sites with a score per site |
| Size | about 1500 lines |

What it shares with this book: the k-mer encoding in [Sequence Encoding](../bio/sequence-encoding.md) is the feature extraction step, and the numerics in [Numerics for Scoring](../bio/numerics.md) are what keeps the scores comparable across windows.

The lesson it produced: a model's input encoding is a program, and the defect classes of chapter 2 apply to it. An off-by-one in the window is a systematically shifted feature, and a model trained on shifted features learns the shift.

## 3. BioCUDA

A planned GPU-accelerated library for the operations in this book: scoring, distance matrices, contact search, and a port of the docking step.

| | |
| --- | --- |
| Language | CUDA C++ |
| Status | in planning; the distance-matrix kernel exists |
| Target | the loops that dominate a virtual screen: pairwise distances and scoring |
| Size | unknown by design |

What it shares with this book: [Parallel Scoring](../bio/parallel-scoring.md) is the CPU version, and the same Amdahl analysis decides whether a GPU port is worth it. A scoring loop that is 60% of the runtime gets at most a 2.5× overall speedup from an infinitely fast GPU.

The lesson so far: measure the serial fraction before writing a kernel. The transfer cost can exceed the compute saved, and the only way to know is the CPU measurement first.

## 4. GROMACS and analysis scripts

A set of standardised GROMACS inputs and post-processing for four system types — protein–ligand, protein–protein, apo protein, and protein–metal — with RMSD, RMSF, \(R_g\), DCCM, MM-PBSA and secondary-structure analysis generated automatically.

| | |
| --- | --- |
| Language | Bash and Python |
| Input | a prepared structure |
| Output | a run directory plus plots |
| Size | about 800 lines |

What it shares with this book: the geometry in [Geometry](../bio/geometry.md) — RMSD, \(R_g\), dihedrals — is the same arithmetic, and the superposition requirement is the one that matters most. A trajectory analysis that reports raw RMSD without superposition reports the molecule drifting, not the molecule changing.

The lesson it produced: an automation script is a program, and its failure modes are the ones in chapter 16 — a missing input, a step that silently produced nothing, and a pipeline that continued anyway.

## 5. What these have in common

All four are the same three layers: a parser that produces an owned model, a numerics or geometry layer that computes on it, and a driver that reports. The layers are separable, testable in isolation, and each has a version in this book that fits in a chapter.

That is the reason the chapters are ordered as they are. A reader who can write the chapter-sized version of each layer can write the project-sized version, because the difference is scale and testing rather than structure.

## Continue

- [How I Use AI to Build a Project](ai-assisted-workflow.md) is the method behind all four.
- [Roadmap](roadmap.md) is what the book still needs.
- [Overview: From Sequence to Structure](../bio/overview.md) is the chapter-sized version of the first layer.
