# About the Author

I keep this page short, because a biography is not an argument. What matters for the book is narrower: which problems I have actually had to solve, because those decide which chapters exist and which Field Notes are mine.

## 1. What I actually build

I am an undergraduate at Henan University of Technology, majoring in Biological Engineering with a minor in Computer Science, and I work where the two meet: bioinformatics, molecular dynamics, enzyme engineering, and the parts of structure analysis that are cheap to state and expensive to compute.

The bioinformatics side is steady work rather than a single project: genome and transcriptome assembly and annotation, differential expression, enrichment, and the conservation and variant scans that come out of an alignment. Most of it never became a paper; it is the throughput that made everything else possible.

Molecular dynamics is where most of my hours go. A set of GROMACS scripts covers four system types — protein–ligand, protein–protein, protein-only, protein–metal — from topology and equilibration through production, and a post-processing toolkit produces RMSD, RMSF, Rg, DCCM, MM-PBSA and secondary-structure plots without manual steps. I care as much about the analysis as the run: a trajectory that has not been reduced to a number is not evidence, and most of the numbers in this book's biology chapters are that kind of reduction.

Enzyme engineering is the application. An automated workflow strings sequence retrieval, multiple sequence alignment, conservation analysis, pocket detection and mutation screening into one run, calling FoldX, DynaMut2 and CataPro and returning a ΔΔG heat map. Site-saturation and alanine scans for activity are the same loop with a different scoring function, and the cases that matter are always the ones where the predicted ΔΔG and the measured activity disagree.

Two Python packages came out of that work and are not published: a high-throughput docking wrapper that goes from pocket detection to AutoDock Vina to PLIP interaction analysis, sized for libraries in the tens of thousands, and a peptide generator for candidate sequences. On the infrastructure side I wrote Protein Phaser, a C++ library that reads PDB and mmCIF files into a chain/residue/atom hierarchy, small enough to embed and strict enough to trust. It became the input layer for a feature engine computing radius-weighted pocket centres, residue depth, SASA and per-residue flexibility, and FlexTrimmer — a model that predicts where a protein can be truncated without losing function — is trained on those features. BioCUDA, still in progress, is the GPU counterpart, and I have written enough CUDA to know which parts of docking and scoring belong on the device and which are simply slower there.

That work is also what I mean by computer-aided drug design: a target chosen from structure and conservation rather than from a database query, pocket and flexibility analysis to decide whether a site is worth screening at all, then docking and interaction analysis as the cheap end of the funnel. None of it has produced a marketed compound, and this book does not pretend otherwise.

## 2. Why I wrote this book

Every one of those projects eventually ran into the same class of problem: a structure parsed correctly but with the wrong padding, a buffer that grew one element too late, an object destroyed while a pointer to it was still alive. The fixes were small. Finding them was not.

What I wanted, and could not find, was a text that put the object model first and the syntax second, and then carried the same model into the domain where it pays off — a PDB file, a distance matrix, a contact map. So the C half of this book is about bytes and lifetimes rather than about statements, and the C++ half assumes that model rather than restarting from "what is a class". The biology section is the test: if the earlier chapters worked, a protein structure is a layout problem and a scoring loop is a numeric one.

The Field Notes are mine. Each one is a defect I actually shipped, with the compiler message or the sanitiser report that finally explained it. I kept them because a defect that looked plausible is more instructive than a defect that looked obviously wrong, and because the alternative — a book of correct examples — would have taught you less about your own code.

## 3. Elsewhere

- Email: [suguohui_account@163.com](mailto:suguohui_account@163.com)
- Main site: [www.huimy.top](https://www.huimy.top)
- Blog: [blog.huimy.top](https://blog.huimy.top)
- Community: [comm.huimy.top](https://comm.huimy.top)
- Curriculum vitae: [www.huimy.top/cv.html](https://www.huimy.top/cv.html)
