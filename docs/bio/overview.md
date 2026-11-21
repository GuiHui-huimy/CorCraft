# Overview: From Sequence to Structure

Before any code, I owe the reader an argument for the order. Nine stages, and I say why each one precedes the next, why I left out the obvious ones such as BLAST and secondary-structure prediction, what the data policy is, and what the three files in every lab are for. If you read one page in this part, read this one; the rest assume it.

The biology half of this book exists for one reason: to give the C and C++ chapters a destination that is not a toy. Everything before this page computed sizes, walked arrays, and fixed bugs in isolation. From here on, the same techniques produce numbers a structural biologist would recognise — a GC fraction, an alignment score, a contact map, a radius of gyration.

This is computational biology, not bioinformatics. The distinction is deliberate and worth one paragraph.

**Bioinformatics** is the management of biological data: formats, databases, queries, annotations, pipelines that call existing tools. It is largely a systems and data-management activity, and it is well served by Python and by tools that already exist.

**Computational biology** here means computing properties of molecules from their coordinates and sequences: the physics and geometry of a structure, the numerics of a scoring function, the cost of evaluating it at scale. It is where arrays, alignment, floating-point behaviour, and memory layout stop being exercises and become the reason the program is slow or wrong.

A structural biologist's daily work is full of the second kind. A PDB file is a text file with fixed columns and a hierarchy of chains, residues and atoms. A contact map is a boolean matrix. A dihedral angle is four points and an arctangent. An RMSD is a sum of squared differences followed by a square root. Each of those is a chapter of this book applied to a real file.

## 1. The nine stages, and the order they come in

| Stage | Topic | What it computes | Chapters it leans on |
| --- | --- | --- | --- |
| 01 | FASTA input | parse headers, validate alphabet | strings, files |
| 02 | Sequence encoding | GC and k-mer counts, 2-bit packing | bit operations, arrays |
| 03 | Numerics | Kahan summation, log-sum-exp | floating point, undefined behaviour |
| 04 | Alignment | Needleman–Wunsch, scoring, traceback | arrays, heap, sorting |
| 05 | PDB parsing | fixed columns, hierarchy, altloc | structs, pointers, translation units |
| 06 | Geometry | distance, dihedral, RMSD, Rg | floating point, numerics |
| 07 | Contacts | contact maps, neighbour search | arrays, sorting, bit ops |
| 08 | Structure model | secondary structure, a C++ model class | classes, RAII, containers |
| 09 | Parallel scoring | OpenMP over the scoring loop | performance, Amdahl |

The order is not arbitrary. Stage 04 needs the numerics from 03 to produce scores that do not drift; stage 06 needs the parser from 05; stage 07 needs the geometry from 06. Skipping ahead is possible but each page says what it assumes.

## 2. Why these and not the obvious ones

Three candidates were deliberately left out.

**No BLAST.** Running an existing aligner is a shell exercise, not a computing one, and the interesting part — why the algorithm is O(nm) and how banded variants work — is stage 04.

**No machine learning framework.** Training a model on sequences is a different book. The closest this text gets is the discussion of why a ResNet-style model is a memory problem before it is a maths problem.

**No database.** Querying UniProt is a network and schema exercise. The one sequence this book embeds — ubiquitin, P0CG48, 76 residues — is short enough to be a constant and long enough to test every parser.

## 3. The data policy

The repository ships no structure files. `data/README.md` lists the three used throughout and the commands to fetch them:

```bash
curl -O https://files.rcsb.org/download/1UBQ.pdb
curl -O https://files.rcsb.org/download/1CRN.pdb
curl -O https://files.rcsb.org/download/1A3N.pdb
```

Three reasons for this policy rather than a vendored copy:

1. Coordinates are large, and a repository that carries them grows without bound.
2. RCSB revises entries; a pinned copy makes the output tests depend on a version that no reader can reproduce from the source.
3. Fetching the file yourself is the first step of your own parser, and it teaches where the data actually comes from.

Stages 05–08 carry their input as string literals, so there is nothing to skip and nothing to download before they run. A missing `data/` file affects only your own prototype, not the reference program.

## 4. The three files, and why each

| File | What it is | Why it is here |
| --- | --- | --- |
| `1CRN` | crambin, 46 residues, 327 atoms | smallest real protein; good for hand-checking distances |
| `1UBQ` | ubiquitin, 76 residues, 602 atoms | the standard small test case; single chain, no gaps |
| `1A3N` | haemoglobin, 4 chains, ~3000 atoms | the multi-chain case: contacts across chains, quaternary structure |

`1CRN` is small enough to compute a full contact map by eye. `1UBQ` is the case every paper uses, so its numbers can be checked against literature. `1A3N` is the case where a parser that assumes one chain breaks.

## 5. What a reader should be able to do at the end

Not "know bioinformatics". Concretely, after stage 09 a reader should be able to:

- Read a PDB file into a structure whose chains, residues and atoms are correct, including the parts most parsers get wrong.
- Compute an RMSD between two conformations and explain why superposition is required for it to mean anything.
- Build a contact map and explain why the naive O(n²) version is acceptable at n = 3000 and not at n = 30000.
- Write a scoring loop, measure it, parallelise it, and report the speedup honestly with the serial fraction stated.
- Explain what a dihedral angle is in terms of four points and an arctangent, without looking it up.

If any of those is still opaque after the corresponding page, the page has failed and the fix belongs in the page, not in a footnote.

## 6. How to read this section

The pages assume the C chapters. A reader who has not done `heap.md` and `structs-and-padding.md` will find stage 05 confusing, because a PDB parser is a struct-layout problem before it is a biology problem. The recommended route in [How to Use This Book](../start-here/how-to-use.md) puts the C chapters first for exactly this reason.

Each page ends with a Practise section pointing at `labs/bio/`, and each reference program states its tolerance in its own assertions.

## Continue

- [FASTA and Sequence Input](fasta.md) is stage 01, the entry point for everything else.
- [PDB Format](pdb-format.md) is the file every structural page reads.
- [Numerics for Scoring](numerics.md) is why the scores do not drift.
