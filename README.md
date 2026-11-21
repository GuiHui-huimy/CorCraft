# C/C++ Craft

A memory-first course in C and C++, ending in protein structure computation.
The book is published at **https://guihui-huimy.github.io/CorCraft/**.

Most material teaches C by describing what a statement does. This repository
teaches it by describing what a statement builds: how many bytes each object
occupies, how long it lives, who owns it, and what happens when the answer
changes. The final section applies that model to PDB parsing, geometry,
contact maps and parallel scoring.

## Layout

| Path     | Contents                                                          |
| -------- | ----------------------------------------------------------------- |
| `docs/`  | The book itself, built with MkDocs Material                       |
| `code/`  | Every example: one directory per chapter, one file per numbered lab |
| `labs/`  | One `check.sh` per stage, building that target and running it     |
| `data/`  | Accession numbers and fetch commands for the reader's own prototypes |
| `kb/`    | A local knowledge base used by the AI-assisted workflow chapter   |
| `tools/` | `Makefile` wrapper and the lab runner                             |

## Build the code

```bash
cmake -S code -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

These commands assume a POSIX shell. Linux or WSL2 is the reference route;
Visual Studio 2022 and a conda-forge toolchain build the same tree, and
`docs/start-here/linux-toolchain.md` gives the flags for each.

Or through the wrapper:

```bash
make -f tools/Makefile test
```

## Run the exercises

```bash
bash tools/run_labs.sh
```

Each lab builds with the project's warning set, `-Wall -Wextra -Wpedantic
-Wconversion -Wshadow` on GCC and Clang, and prints any warning it sees. Zero
warnings is the rule for a chapter's code, checked in review rather than by
`-Werror`, so that a reader can still run a lab that warns and learn why.

## Serve the book

```bash
python3 -m pip install --user mkdocs-material
mkdocs serve
```

## Conventions

- Every source file opens with a `LAB` line, a `Drills` line and a `Build` line.
- Every example is a single self-contained translation unit with its own `main`.
- No example writes into the source tree; binaries go to `build/` or `/tmp`.
- A page never claims behaviour the source does not exhibit.

## Author

Mr. Su — Biological Engineering (major), Computer Science (minor), Henan
University of Technology. Contact: `suguohui_account@163.com`.
