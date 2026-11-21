# About the Author

I keep this page short, because a biography is not an argument. What matters for the book is narrower: which problems I have actually had to solve, because those decide which chapters exist and which Field Notes are mine.

## 1. What I actually build

I am a undergraduate at Henan University of Technology, majoring in Biological Engineering with a minor in Computer Science, and I work where the two meet: computational biology, with an emphasis on rational enzyme design, molecular dynamics, and making simulation workloads finish sooner.

My projects come out as pipelines rather than papers. An automated enzyme-engineering workflow strings sequence retrieval, multiple sequence alignment, conservation analysis, pocket detection and mutation screening into one run, calling FoldX, DynaMut2 and CataPro along the way and returning a ΔΔG heat map at the end. A virtual-screening pipeline goes from pocket detection to AutoDock Vina docking to PLIP interaction analysis, sized for libraries in the tens of thousands. A set of GROMACS scripts covers four system types — protein–ligand, protein–protein, protein-only, protein–metal — and a post-processing toolkit produces RMSD, RMSF, Rg, DCCM, MM-PBSA and secondary-structure plots without manual steps.

On the infrastructure side I wrote Protein Phaser, a C++ library that reads PDB and mmCIF files into a chain/residue/atom hierarchy, small enough to embed and strict enough to trust. It later became the input layer for a feature engine computing radius-weighted pocket centres, residue depth and SASA. BioCUDA, still in progress, is its GPU counterpart: an attempt to move the expensive parts of docking onto the device.

Some of this work also appears as writing: a tutorial on the statistical mechanics behind molecular dynamics, a batch RESP charge workflow built on ORCA and MultiWFN, and FlexTrimmer, a model that predicts where a protein can be truncated without losing function.

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
