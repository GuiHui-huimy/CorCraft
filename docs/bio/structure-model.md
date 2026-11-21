# Structure Model

A structure is an ownership graph before it is a data structure, and the page is about who frees what. I draw the graph, then load with an error path that returns instead of exiting, then the queries a caller actually makes, then what the C++ version buys — destructors, mostly — then the guard that only guarded half the cases.

The C parser in stage 05 produced arrays and pointers. This stage produces the same data as an object graph, because the operations a reader actually wants — select a chain, find a residue, iterate over atoms — are all queries that a set of parallel arrays makes awkward and a hierarchy makes natural.

## 1. The ownership graph

```text
Structure  owns  std::vector<Chain>
Chain      owns  std::vector<Residue>
Residue    owns  std::vector<Atom>
Atom       owns  nothing (plain data)
```

```cpp
// LAB 08-01  bio_model/model.cpp
struct Atom {
    std::string name;
    char altloc{};
    char element[3]{};
    double x{}, y{}, z{};
    float occupancy{}, bfactor{};
};

class Residue {
public:
    char chain{};
    int seq{};
    char icode{};
    std::string name;
    std::vector<Atom> atoms;
};

class Chain {
public:
    char id{};
    std::vector<Residue> residues;
};

class Structure {
public:
    std::vector<Chain> chains;
    static std::optional<Structure> load(std::istream &in);
    std::size_t atom_count() const;
    const Residue *find(char chain, int seq, char icode) const;
};
```

Every level owns its children, so destruction is automatic and there is no `free` anywhere. The graph is a tree of vectors, which means the memory is contiguous at each level: a chain's residues are one block, a residue's atoms are one block. That is better for iteration than a linked structure and worse for insertion, which is the right trade for a file parsed once.

`Atom` holds a `std::string name` — five bytes of content in a 32-byte object on a 64-bit system. That is 27 bytes wasted per atom, 3000 atoms, 80 KB. The alternative is a `char name[5]`, which costs the ability to compare with `==` but saves nothing else; for a structure of this size it is not worth the change. For a 100 000-atom model the decision flips.

## 2. Loading, and where the error goes

```cpp
std::optional<Structure> Structure::load(std::istream &in) {
    Structure s;
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() < 54) { continue; }              // too short to be ATOM
        if (line.compare(0, 6, "ATOM  ") != 0 &&
            line.compare(0, 6, "HETATM") != 0) { continue; }
        Atom a;
        a.name = trim(line.substr(12, 4));
        a.altloc = line[16];
        a.x = std::stod(line.substr(30, 8));
        a.y = std::stod(line.substr(38, 8));
        a.z = std::stod(line.substr(46, 8));
        s.attach(line[21], std::stoi(line.substr(22, 4)), line[26], a);
    }
    if (s.chains.empty()) { return std::nullopt; }
    return s;
}
```

`substr` with a column offset and a width is the C++ form of the fixed-column read, and it is the same discipline as the C page: the offset and the width come from the format table, not from a delimiter.

`std::stod` throws on malformed input, which is the wrong behaviour for a parser reading an untrusted file: one bad coordinate should reject the file, not unwind. The lab's version wraps the conversions in a try block at the load boundary and converts to `std::nullopt`, which is the explicit translation from "the library's error model" to "this parser's error model".

`attach` does the hierarchy work: find or create the chain, find or create the residue keyed by `(seq, icode)`, then append the atom. In a vector-based graph this is a linear search per atom, which is O(1) amortised in practice because consecutive atoms belong to the same residue and the search starts at the end.

```cpp
void Structure::attach(char chain_id, int seq, char icode, Atom a) {
    if (chains.empty() || chains.back().id != chain_id) { chains.push_back(Chain{chain_id}); }
    auto &c = chains.back();
    if (c.residues.empty() || c.residues.back().seq != seq || c.residues.back().icode != icode) {
        c.residues.push_back(Residue{chain_id, seq, icode, ""});
    }
    c.residues.back().atoms.push_back(std::move(a));
}
```

The assumption — atoms arrive grouped by chain and residue, in order — is a property of the PDB format and is stated in the comment. A parser that assumed it without stating it would be wrong for a file that interleaves chains, which some files do. The exercise is to feed the same structure in interleaved order and require the model to come out identical, which forces a real lookup rather than a `back()` check.

## 3. Queries

```cpp
const Residue *Structure::find(char chain, int seq, char icode) const;
std::size_t Structure::atom_count() const;      // sum over chains and residues
```

The `find` is O(chains + residues) with the linear scan. For repeated lookups the lab adds an index:

```cpp
std::unordered_map<key, const Residue *> index_;   // key is (chain, seq, icode) packed
```

The pointer is valid as long as the structure is not modified — which the model guarantees by keeping `chains` private after load and exposing only const access. This is the RAII argument applied to a query: the index borrows, the structure owns, and the borrow cannot outlive the owner because the owner cannot change.

## 4. What the C++ version buys

| | C version (stage 05) | C++ model (stage 08) |
| --- | --- | --- |
| Allocation | one `malloc` per level, `cap` fields | automatic, growth amortised |
| Destruction | three `free` functions, called by the caller | destructors |
| Error path | return code checked at every call | one `optional` at the boundary |
| Iteration | index arithmetic on parallel arrays | range-for over vectors |
| Copy | hand-written deep copy or none | implicit, deep, correct |

The last row is the one that matters most: a C structure with three owned pointers has no correct implicit copy, so every caller must either avoid copying or write a deep copy. The C++ version's default copy is deep and correct without being written.

## 5. Field note: the guard that was only half a guard

The archived `CPU.h` began:

```cpp
#ifndef CPU_H
#pragma once
class CPU { ... };
#endif
```

There is no `#define CPU_H`. The guard therefore never becomes active, and `#pragma once` is doing all the work — which it does correctly on GCC, Clang and MSVC. The problem appears the day the file is included through a path that the compiler considers distinct (a symlink, a case-insensitive filesystem with two spellings, a build system that copies headers), because `#pragma once` is keyed on the file identity and the guard is keyed on the macro.

The file worked for a year, and the defect was one line: `#define CPU_H` after the `#ifndef`. The generalisation: a guard is a three-line construct and an incomplete one is worse than none, because it looks like protection. The repository's headers use both the macro and `#pragma once`; a one-line `grep` for `#ifndef` without a matching `#define` is what catches the half-written guard, and it belongs in review rather than in `check.sh`, because a header that no lab includes is still a header.

The same file had `person.h` with the constructor body inside the class — the defect the CMake chapter describes — so the project had two headers with two different incomplete mechanisms and neither was sufficient.

## 6. Practise

Build `code/bio/08_structure_model/model.cpp` and print the atom count, chain count and residue count for the three structures. Then attempt the exercise in `labs/bio/08_structure_model/`, which requires the parser to produce identical results for a chain-interleaved file and a chain-ordered file, and to hold a residue index that stays valid through a full pass.

## Continue

- [PDB Format](pdb-format.md) is the format this model represents.
- [RAII and Smart Pointers](../cpp/raii.md) is the ownership discipline behind the destructors.
- [Contacts](contacts.md) is the query this model makes cheap.
