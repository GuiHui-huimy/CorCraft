# About the Author

I keep this page short, because a biography is not an argument. What matters for the book is narrower: which problems I have actually had to solve, because those decide which chapters exist and which Field Notes are mine.

## 1. What I actually build

I am an undergraduate at Henan University of Technology, majoring in Biological Engineering with a minor in Computer Science, and I work where the two meet: genome and transcriptome analysis, differential expression, molecular dynamics, and the structure work that is cheap to state and expensive to compute. My GROMACS pipelines cover four system types — protein–ligand, protein–protein, protein-only, protein–metal — from topology and equilibration through production, and a post-processing toolkit turns trajectories into RMSD, RMSF, Rg, DCCM, MM-PBSA and secondary-structure plots without manual steps. A trajectory that has not been reduced to a number is not evidence.

Enzyme engineering is where that lands: an automated workflow runs sequence retrieval, alignment, conservation analysis, pocket detection and mutation screening in one pass, calling FoldX, DynaMut2 and CataPro and returning a ΔΔG heat map, with site-saturation scans for activity as the same loop and a different scoring function. Two Python packages came out of it and are not published — a high-throughput docking wrapper (pocket, AutoDock Vina, PLIP) and a peptide generator. Protein Phaser, a C++ library that reads PDB and mmCIF into a chain/residue/atom hierarchy, became the input layer for a feature engine computing pocket centres, residue depth, SASA and per-residue flexibility, and FlexTrimmer is trained on those features; BioCUDA is its GPU counterpart, and I have written enough CUDA to know which parts belong on the device. None of it has produced a marketed compound, and this book does not pretend otherwise.

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
