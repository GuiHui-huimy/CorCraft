<p align="center">
  <a href="https://guihui-huimy.github.io/CorCraft/" target="_blank">
    <img src="./docs/assets/images/Logo.png" width="640" alt="C/C++ Craft — logo placeholder">
  </a>
</p>

<!-- Placeholder: replace the source above with docs/assets/images/logo.png when the logo exists. -->

<p align="center">
  <strong>
    C/C++ Craft — Memory-First C and C++, from a Byte to a Protein Structure
  </strong>
</p>

<p align="center">
  A teaching repository, not a reference manual: every claim about an object is
  checked against its size, its lifetime and its owner.
</p>

<p align="center">
  <a href="https://en.cppreference.com/w/c/17" target="_blank">
    <img src="https://img.shields.io/badge/C-C17-00599C?logo=c&amp;logoColor=white" alt="C17">
  </a>
  <a href="https://en.cppreference.com/w/cpp/20" target="_blank">
    <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&amp;logoColor=white" alt="C++20">
  </a>
  <a href="https://cmake.org/" target="_blank">
    <img src="https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake&amp;logoColor=white" alt="CMake">
  </a>
  <a href="https://squidfunk.github.io/mkdocs-material/" target="_blank">
    <img src="https://img.shields.io/badge/Docs-MkDocs%20Material-526CFE" alt="MkDocs Material">
  </a>
  <a href="docs/start-here/linux-toolchain.md" target="_blank">
    <img src="https://img.shields.io/badge/Build-Linux%20%7C%20VS%202022%20%7C%20conda-2E7D32" alt="Build routes">
  </a>
  <img src="https://img.shields.io/badge/Status-Complete-brightgreen" alt="Status">
</p>

<p align="center">
  Built with C17, C++20, CMake and MkDocs Material. Linux or WSL2 is the
  reference build route; Visual Studio 2022 and a conda-forge toolchain build
  the same tree with their own flags. Thirty-eight stage directories are
  registered as 65 separate tests, and the final section turns the memory
  model into PDB parsing, geometry, contact maps and parallel scoring.
</p>

<h2></h2>

Most material teaches C by describing what a statement does. This repository
teaches it by describing what a statement builds: how many bytes each object
occupies, how long it lives, who owns it, and what happens when the answer
changes. The book is published at **https://guihui-huimy.github.io/CorCraft/**.

## Layout

| Path     | Contents                                                             |
| -------- | -------------------------------------------------------------------- |
| `docs/`  | The book itself, built with MkDocs Material                          |
| `code/`  | Every example: one directory per chapter, one file per numbered lab  |
| `labs/`  | One `check.sh` per stage, building that target and running it        |
| `data/`  | Accession numbers and fetch commands for the reader's own prototypes |
| `kb/`    | A local knowledge base used by the AI-assisted workflow chapter      |
| `tools/` | `Makefile` wrapper and the lab runner                                |

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

## Licence

The text is CC BY-NC-SA 4.0; the code in `code/`, `labs/` and `tools/`, and every code block inside a page, is MIT. The names, "C/C++ Craft" and "CorCraft", and the logo are reserved. `LICENSE` holds the MIT text, `NOTICE` lists the third-party components, and `docs/license.md` states the scope table, the attribution format and the commercial-permission route.

## Author

Mr. Su — Biological Engineering (major), Computer Science (minor), Henan
University of Technology. Contact: `suguohui_account@163.com`.
