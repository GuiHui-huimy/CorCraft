# PDB Format

PDB is a fixed-column format pretending to be whitespace-separated, and every parser that splits on spaces is a future bug. I mark the columns of one record, then show the split failing, then read coordinates, then build the hierarchy of model, chain, residue, atom, then decide what to do with `HETATM`. Everything is a struct because a 30 MB file is not a place for per-atom allocation.

The Protein Data Bank format is a fixed-column text format from 1971. Every field has a column range, not a delimiter. This is the single most important fact about the format, and it is the reason most parsers of it are subtly wrong.

## 1. A record, with columns marked

```text
         1         2         3         4         5         6         7         8
12345678901234567890123456789012345678901234567890123456789012345678901234567890
ATOM      1  N   MET A   1      27.340  24.430   2.614  1.00  9.19           N
```

| Field | Columns | Content |
| --- | --- | --- |
| record name | 1–6 | `ATOM  ` or `HETATM` |
| serial | 7–11 | atom serial number |
| atom name | 13–16 | ` N  `, ` CA `, with the element right-aligned |
| altLoc | 17 | alternate conformation indicator |
| residue name | 18–20 | `MET` |
| chain ID | 22 | `A` |
| residue sequence | 23–26 | `   1` |
| insertion code | 27 | ` ` or `A` |
| x, y, z | 31–38, 39–46, 47–54 | `%8.3f` |
| occupancy | 55–60 | `%6.2f` |
| temperature factor | 61–66 | `%6.2f` |
| element | 77–78 | ` N`, ` C` |

The columns are the contract. A file may contain spaces inside a field, and the fields are separated by nothing in particular — so tokenising on whitespace is wrong in a way that usually works.

## 2. Why whitespace splitting fails

```c
char *tok = strtok(line, " \t");
```

Three failures, all real:

1. **The atom name field has leading spaces by design.** ` N  ` and ` CA ` are both four columns, and the alignment encodes whether the element is one or two letters. Splitting drops that information, so a parser cannot distinguish a calcium atom `CA` from an alpha carbon ` CA ` without guessing from the residue name.
2. **A blank chain ID collapses the token stream.** With no chain, columns 22 is a space, and splitting produces one fewer token than the parser's field table expects — every subsequent field shifts.
3. **Residue names with spaces** (`  A` for a nucleic acid) and negative occupancies break the assumption further.

The format's designers chose fixed columns precisely so that a blank field is still a field.

## 3. Reading the coordinates

```c
/* LAB 05-01  bio_pdb/pdb_dump.c */
double x, y, z;
/* the substring, not the whole line */
char xs[9] = {0}, ys[9] = {0}, zs[9] = {0};
memcpy(xs, line + 30, 8);
memcpy(ys, line + 38, 8);
memcpy(zs, line + 46, 8);
x = atof(xs); y = atof(ys); z = atof(zs);
```

`%8.3f` guarantees three decimal places and at most eight columns; `atof` on the copied substring handles the sign and the decimal point. `sscanf(line + 30, "%8lf%8lf%8lf", &x, &y, &z)` is also correct and avoids the copy, because the field width limits the conversion — the width is the part that matters, not the copy.

```c
sscanf(line + 30, "%8lf", &x);      // correct: width-limited
sscanf(line, "%*31s %lf", &x);      // wrong: relies on whitespace
```

The wrong form works on well-formed files and fails on any file where a coordinate has a leading space and the previous field is short.

## 4. The hierarchy

```text
Structure
└── Chain (A, B, C, ...)
    └── Residue (identified by sequence number + insertion code)
        └── Atom (identified by name + altLoc)
```

The residue identity is `(chain, resSeq, iCode)`, not `resSeq`. Two residues in the same chain can share a sequence number when an insertion code distinguishes them, which happens in real structures. A key that uses only `resSeq` merges them.

The atom identity is `(name, altLoc)`. Alternate conformations are labelled `A`, `B`, ... in column 17, and a structure with two conformations has two atoms named `CA` in the same residue. A parser that keeps the first one and ignores the rest is a parser that silently drops half the model.

```c
typedef struct {
    char name[5];
    char altloc;
    char element[3];
    double x, y, z;
    float occupancy, bfactor;
} atom;

typedef struct {
    char name[4];
    char icode;
    int seq;
    atom *atoms;
    size_t n_atoms, cap;
} residue;
```

The `altloc` field is one `char` and costs nothing; omitting it costs the ability to represent a legal file.

## 5. `ATOM` versus `HETATM`, and what to keep

`ATOM` records are polymer atoms; `HETATM` are ligands, waters, and modified residues. A structure viewer usually shows both; a contact map usually wants only the polymer. The distinction must be stored, not applied at parse time, because different consumers want different subsets.

`TER` marks the end of a chain in older files; `END` marks the end of the model. Files with multiple models (`MODEL`/`ENDMDL`) exist — an NMR ensemble has twenty — and the parser must either read only the first model or index the rest explicitly.

## 6. Size, and why everything is a struct

```c
sizeof(atom)      /* 64 bytes: 5 + 1 + 3 + padding + 3 doubles + 2 floats */
```

`1UBQ` has 602 atoms, so the atom array is 38 KB — smaller than the file it came from, and comfortably in L2. `1A3N` has about 3000 atoms, 190 KB. Neither is a memory problem, which is why the parser can afford one struct per atom and the coordinates can be `double` rather than `float`.

The struct is 64 bytes because of alignment: five chars plus padding to reach the first `double` at offset 8, and the two floats padded to the end. Chapter 6's layout rules apply directly, and a parser that reorders the members to `{double x, y, z; float occupancy, bfactor; char name[5]; char altloc; char element[3];}` gets 48 bytes — same data, 25% less memory, and a different cache behaviour on the iteration that touches only coordinates.

## 7. Field note: the parser that read the wrong column

A course program parsed atom records like this:

```c
sscanf(line, "%6s%5d %4s %3s %c%4d %8lf%8lf%8lf",
       rec, &serial, atom_name, res_name, &chain, &res_seq, &x, &y, &z);
```

It worked on `1CRN` and failed on `1A3N`. The cause: the format's spaces between fields are not guaranteed to be single, and `%4s` for the atom name consumes ` N  ` correctly but for ` CA ` consumes `CA` and leaves the space to be eaten by the next specifier — which happens to be fine. The failure came one field earlier: `%3s` for the residue name matched `MET` and then `%c` read the space in column 21 rather than the chain in column 22, because the format has a two-column gap between residue name (18–20) and chain (22) that the `sscanf` format string did not account for.

The chain came out as `' '` for every residue, so all four chains of haemoglobin merged into one, and the quaternary contacts were computed within a single merged chain.

The repair is the one this chapter keeps repeating: parse by column offset, and let the field widths enforce the widths. A format defined by columns cannot be parsed by delimiters, and a `sscanf` format that does not encode the gaps is a delimiter parser in disguise.

## 8. Practise

Build `code/bio/05_pdb_parse/pdb_dump.c` and run `bash labs/bio/05_pdb_parse/check.sh`. The program parses an embedded five-line fragment — four `ATOM` records on two chains and one `HETATM` — and asserts the chain, residue and `HETATM` counts, which is the multi-chain case a naive counter gets wrong. Then download the three files and repeat the count on `1CRN`, `1UBQ` and `1A3N` (327, 602 and about 3000 atoms); that is the extension, and it is where `data/README.md` matters.

## Continue

- [Geometry](geometry.md) is what the coordinates are for.
- [Structure Model](structure-model.md) turns the parse into an owning C++ class.
- [Structs and Padding](../c/structs-and-padding.md) is why the atom struct is 64 bytes.
