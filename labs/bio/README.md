# Computational Biology Labs

These labs follow the same layout — one directory per stage, one `check.sh` —
but they are written against real biological data: a PDB fragment, a FASTA
record, a scoring matrix. Nothing is downloaded and nothing is committed as a
data file: every program in `code/bio/` carries its input as a string literal,
so the numbers in the chapter are reproducible on a machine with no network.
`data/README.md` holds the accession numbers for the reader's own prototypes,
the ones that do read a file.

```
labs/bio/<stage_number>_<stage_name>/
└── check.sh      builds one target with the project flags, runs it, exits 0
```

Judgement is by tolerance, not by text. A program that prints a distance is
checked on `|computed - reference| < 1e-3`, and the log-sum-exp lab on a
relative error against a `long double` reference; the last digit of a formatted
`double` is an artefact of the platform, not of the algorithm.

`09_parallel/omp_scores.c` is the one lab whose behaviour depends on the
compiler rather than on the code: with OpenMP found, it runs four threads and
prints the thread count; without it, the same source runs serially and prints
`serial`. Both are a pass — the chapter's measurement is about the reduction
being order-independent, not about the machine having four cores.

Run every lab at once from the repository root:

```bash
bash tools/run_labs.sh
```
