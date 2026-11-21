# Sample Data

No coordinate files are stored in this repository. The programs under `code/bio/` carry their inputs as literals so that every number in the chapter is reproducible without a download; the files below are for the reader's own prototypes, the ones that read a structure from disk.

## Structures

| Purpose | Accession | Command |
| --- | --- | --- |
| Small, well-ordered protein; the default first parse | `1UBQ` | `curl -O https://files.rcsb.org/download/1UBQ.pdb` |
| 46-residue crambin; tiny enough to check by hand | `1CRN` | `curl -O https://files.rcsb.org/download/1CRN.pdb` |
| Multi-chain assembly for a contact map | `1A3N` | `curl -O https://files.rcsb.org/download/1A3N.pdb` |

Place the files in this directory. A prototype you write after chapter 05 reads
`../data/1UBQ.pdb` relative to its own directory; the reference program in
`code/bio/05_pdb_parse/` embeds a five-line fragment instead — four `ATOM`
records and one `HETATM`.

## Sequences

For a FASTA prototype, UniProt `P0CG48` is the ubiquitin sequence, 76 residues:

```
>sp|P0CG48|UBC_HUMAN Ubiquitin
MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQRLIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG
```

Save it as `ubiquitin.fasta`. A second file with two records tests multi-record
parsing; build it by repeating the same sequence under two headers.

## Why not ship the files

Public structures change revision, and a committed copy silently diverges from
the database. Downloading on demand keeps the reference current and keeps the
repository small.
