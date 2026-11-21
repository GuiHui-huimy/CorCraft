# Repository Layout

The tree is a contract, and this page states the four rules that make it one: numbered stage directories, a lab number in every source file's header, one `check.sh` per stage that builds one target, and a four-line README. Then the order for registering a new lab, then what I deliberately left out and why.

Where everything lives, and the four naming rules that keep the book, the code and the tests pointing at each other.

## 1. The tree

```text
CorCraft/
├── mkdocs.yml                  site configuration (theme section untouched)
├── docs/
│   ├── start-here/             preface, author, toolchain, first program, how to use
│   ├── c/                      16 chapters, memory-first
│   ├── cpp/                    13 chapters, one Practise section each
│   ├── engineering/            5 chapters on build, test, measure, debug
│   ├── bio/                    10 chapters, sequence to structure
│   ├── reference/              6 lookups
│   ├── about/                  roadmap, projects, AI workflow
│   ├── material/overrides/     home.html and theme overrides
│   ├── assets/                 images, favicon
│   └── includes/mkdocs.md      snippets appended by the snippet extension
├── code/
│   ├── CMakeLists.txt          the single build entry point
│   ├── include/                craft_assert.h, craft_bench.h, craft_io.h
│   ├── c/01_objects_in_memory/ … 16_/error_handling/
│   ├── cpp/01_bridge/ … 13_capstone/
│   └── bio/01_fasta/ … 09_parallel/
├── labs/
│   ├── c/  cpp/  bio/          one check.sh per lab, one directory per stage
├── data/                       README with fetch commands; coordinates not committed
├── kb/                         local knowledge base notes (see kb/README.md)
└── tools/
    ├── Makefile                configure / build / test / sanitize / labs / clean
    └── run_labs.sh             runs every labs/*/*/check.sh
```

## 2. Rule 1: a stage directory is numbered and named

```text
code/c/07_heap/               documents a topic
code/bio/05_pdb_parse/        documents a topic
```

The number matches the chapter order and appears in the chapter's path. A stage added between two existing ones gets a decimal (`07a_`), not a renumbering — renumbering breaks every `LAB` identifier in the prose and in the tests.

## 3. Rule 2: a source file carries its lab number

```text
code/c/07_heap/01_growth_trace.c      LAB 07-01
code/cpp/05_polymorphism/02_slicing.cpp   LAB 05-02
```

The two-level number in the file name is `stage-file`. `LAB 07-02` in the prose refers to exactly one file, and the file's header comment repeats it:

```c
/* LAB 07-02  heap/02_double_free.c
 * Drills: the second free, and why setting the pointer to NULL is the guard.
 * Build:  cmake --build build --target c_07_double
 */
```

Three labelled lines, always in that order, because the reader who lands in a file mid-chapter needs the link back to the prose and the reader in the prose needs the one command that produces the binary. A stage that needs a sentence of context puts it after a blank line, as `02_double_free.c` does for its `--unsafe` mode.

## 4. Rule 3: the acceptance script is named `check.sh` and builds one target

```text
labs/cpp/13_capstone/
└── check.sh        configures `code/`, builds that target, runs it, exits 0
```

The directory holds one file on purpose. The exercise is in `code/<track>/<stage>/README.md` and in the chapter's Lab section; a `starter.c` beside a working `solution.c` teaches reading, not writing, and two copies of the same program drift apart. `check.sh` is the regression harness: it proves the reference implementation still produces the stated result under the flags the chapter quotes, so that when a reader's own attempt fails, the toolchain is already ruled out.

No lab depends on a downloaded file. Every program in `code/bio/` carries its coordinates or sequences as a literal, so `check.sh` never skips and never reports a missing `data/` file as a lab failure. The reader's later prototypes — the ones that do read a PDB file — are the place where `data/README.md` matters.

## 5. Rule 4: one README per stage, four lines

```text
# 07_heap

Drills: realloc growth, double free, leak hunt.
Chapter: docs/c/heap.md
Labs: `01_growth_trace.c`, `02_double_free.c`, `03_leak_hunt.c`
```

The README is the index for a directory a reader may land in without the prose. The `Labs:` line lists the files that are actually in the directory, not the ones intended for it — a README that advertises a file the repository does not contain is the cheapest possible broken link.

## 6. Registering a new lab, in order

1. Write the source in the stage directory with its sequence number.
2. Add one line to the stage's `CMakeLists.txt`: `craft_lab(c_07_growth 07_heap/01_growth_trace.c)`.
3. Write the chapter paragraph that references `LAB 07-02` and the file path.
4. Add the `check.sh` under `labs/<track>/<stage>/`, naming the same target.
5. Add the chapter to `mkdocs.yml`'s nav.
6. Run `make test` and `tools/run_labs.sh`.

Steps 2 and 3 were the ones skipped in the first draft of this repository, and the result was prose that referenced files which did not exist. The order above exists because the reference is a link and a link must resolve.

## 7. What is deliberately absent

- **No vendored structure files.** `data/README.md` explains why and gives the fetch commands.
- **No pre-built documentation site in the repository.** The site is generated from `docs/`; committing `site/` would double every edit.
- **No theme fork.** `docs/material/overrides` holds `home.html` and a few partials; the Material theme itself is a dependency, not a copy.
- **No license file yet.** The repository has not declared one, and the pages do not claim one. Adding it is a decision for the owner, not a default.

## Continue

- [Contributing](../contributing.md) is the workflow these rules belong to.
- [How to Use This Book](../start-here/how-to-use.md) explains the lab protocol.
- [Glossary](glossary.md) defines the terms used in the rules above.
