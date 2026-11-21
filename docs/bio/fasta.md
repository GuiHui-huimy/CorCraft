# FASTA and Sequence Input

A FASTA file looks like the easiest format in biology, and that is exactly why the three promises it does not make will bite you. I parse it line by line without a fixed buffer, then attack line endings, then write the C++ version to show what changes when a `std::string` owns the length, then argue about how strict validation should be. The field note is a counter that counted a header as a sequence.

A FASTA file is a header line beginning with `>` and one or more lines of residues. That is the entire specification. Everything difficult about parsing one comes from what real files contain beyond the specification: Windows line endings, blank lines, lowercase residues, trailing whitespace, comments, and duplicate identifiers.

## 1. The format, and the three things it does not promise

```text
>sp|P0CG48|UBC_HUMAN Ubiquitin OS=Homo sapiens OX=9606 GN=UBC PE=1 SV=1
MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQRLIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG
```

What the format promises: the header starts with `>`, and residues follow until the next `>` or the end of file.

What it does not promise:

1. **Line length.** Sixty is a convention, not a rule. A single 76-character line and 76 one-character lines are both valid.
2. **Case.** Lowercase residues mean the same amino acid and are common in soft-masked alignments.
3. **Uniqueness.** Two records may share an identifier. A parser that stores sequences in a map keyed by identifier silently loses one.

None of these are exotic. All three appear in real downloads, which is why a parser that assumes them is a parser that works on the tutorial file and fails on the user's file.

## 2. The parser

```c
/* LAB 01-01  bio_fasta/fasta_c.c */
typedef struct {
    char *name;        /* owned */
    char *residues;    /* owned, NUL-terminated */
    size_t length;
} record;

typedef struct {
    record *items;     /* owned */
    size_t count;
    size_t capacity;
} record_list;
```

Three ownerships, three frees, in one struct — this is the chapter 7 obligation in its natural habitat. The `capacity` field is the vector growth pattern from chapter 7, written by hand because this stage is C.

```c
static int is_residue(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '*';
}
```

`'*'` is the stop codon, present in translated sequences. Rejecting it fails on real files; accepting it and storing it as a residue fails on the geometry later. The decision this book makes: accept it in the parser, and let the consumer decide. A parser that cannot represent a legal input is a parser with a hidden assumption.

## 3. Reading by line without a fixed buffer

```c
char *line = NULL;
size_t cap = 0;
ssize_t n;
while ((n = getline(&line, &cap, fp)) != -1) { ... }
free(line);
```

`getline` is POSIX, not C standard, and it is the only correct way to read a line of unknown length. The C-standard alternative is a fixed buffer with `fgets`, which truncates a long line into two records without saying so — the defect class of chapter 5, applied to a file format.

Route B readers do not have `getline`. The MSVC equivalent is `fgets` in a loop with a length check, or `std::getline` if the stage is built as C++. The chapter note on that page says so; the C version keeps a `#ifdef _WIN32` branch with the loop, so the lab compiles on both.

```c
/* the _WIN32 branch: read until the newline or the buffer is full */
char buf[512];
while (fgets(buf, sizeof buf, fp)) {
    size_t len = strlen(buf);
    if (len == sizeof buf - 1 && buf[len - 1] != '\n') { /* line continues */ }
}
```

The `len == sizeof buf - 1` test is the only reliable way to detect truncation with `fgets`, because a line of exactly that length and a truncated line are otherwise indistinguishable.

## 4. Line endings

A file saved on Windows ends its lines with `\r\n`. Inside a C string read by `getline`, the `\r` is part of the line, and it lands at the end of the residue string:

```c
"MQIFVKT\r"    /* length 9, last character is not a residue */
```

The defect surfaces much later, as a length that is one too large, or as a residue that fails `is_residue`. The repair is to strip a trailing `\r` at read time, in one place:

```c
if (len > 0 && line[len - 1] == '\r') { line[--len] = '\0'; }
```

Stripping in one place rather than in every caller is the difference between a parser and a set of functions that happen to read lines.

## 5. The C++ version, and what changes

```cpp
// LAB 01-02  bio_fasta/fasta_cpp.cpp
std::optional<Sequence> parse(std::string_view header, std::string_view body);
```

The C++ parser keeps the same record structure and replaces the manual growth with a `std::vector`. The interesting change is `std::string_view` for the body: parsing does not need to own the characters, only to read them, so the signature says so and the caller keeps the buffer alive.

```cpp
for (char c : body) {
    if (c == '\n' || c == '\r' || c == ' ') { continue; }
    if (!is_residue(c)) { return std::nullopt; }
    out.push_back(c);
}
```

`std::nullopt` rather than an exception, because a malformed record is an expected input, not an exceptional condition — chapter 12's distinction applied at a file boundary.

## 6. Validation, and how strict

| Check | Strict? | Why |
| --- | --- | --- |
| Header starts with `>` | yes | without it the record is unparseable |
| Residues are letters or `*` | yes | a digit or a `-` is a gap or a corruption, not a residue |
| Empty record | reject | an empty sequence divides by zero at the GC step |
| Duplicate identifier | accept, report | legal, but a caller may want to know |
| Lowercase | accept, normalise | legal and meaningful |
| Whitespace inside a line | strip | legal per the format, common in wrapped output |

The one that surprises people is the empty record. A header with no residues is syntactically valid, and it produces a zero-length sequence that every downstream fraction divides by. Rejecting it at the boundary is cheaper than a guard in every consumer.

## 7. Field note: the counter that counted a header

A course exercise read a sequence file and printed the GC fraction by counting every `'G'` and `'C'` in the file — including the characters in the header line, where `Homo sapiens` contains a capital `G` in `Homo`? It does not, but `GN=UBB` does, and the header of one test file contained `G` twice. The reported GC fraction was 3 percentage points off on a 76-residue sequence, and the error looked like a rounding problem.

The defect is the absence of state: the reader had no notion of "currently inside a header" versus "currently reading residues", so the format's two line types were the same line type.

```c
int in_header = 0;
while (getline(...)) {
    if (line[0] == '>') { in_header = 1; continue; }
    if (in_header) { in_header = 0; }
    /* only here are the characters residues */
}
```

The generalisation: a file format with two record types requires the parser to have a state, and a parser that applies one rule to every line is not parsing.

## 8. Practise

Build `code/bio/01_fasta/fasta_c.c` and run it: it parses the ubiquitin record in `data/README.md` — 76 residues, with CRLF endings, a wrapped body and a lowercase record all accepted — and rejects three malformed inputs (no `>`, a digit in the body, an empty record). Attempt the exercise in `labs/bio/01_fasta/`, whose task is `fasta_cpp.cpp`: the same parser in C++ with the length and header assertions carried over.

## Continue

- [Sequence Encoding](sequence-encoding.md) consumes the residues this page produces.
- [Numerics for Scoring](numerics.md) turns compositions into numbers.
- [PDB Format](pdb-format.md) is the other file format, with a hierarchy the FASTA format does not have.
