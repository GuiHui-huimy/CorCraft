# Changelog

Changes to this book and to the code it ships with. Entries are dated; a page change names the chapter, a code change names the directory.

## 2026-11-21 — Repository initialised

### Documentation

- Added `start-here/preface.md`, stating the memory-first argument and the six-section structure.
- Added `start-here/about-the-author.md` with a short biography and four contact routes.
- Added `start-here/linux-toolchain.md`: three routes (Linux or WSL2, Visual Studio 2022, conda-forge), version floors, a sanitiser runtime check, and six common installation failures.
- Added `start-here/first-program.md`: one program compiled four times, with the warning text quoted verbatim.
- Added `start-here/how-to-use.md`: numbering, the four-line header, `ctest`, the lab protocol, three rules for using a model.
- Replaced `contributing.md`, `security.md` and this file, which still described the inherited template project.
- Set the site identity in `mkdocs.yml`: name, URL, repository, edit URI, copyright, magic-link repository and the GitHub social link.
- Replaced the homepage text nodes with the book's own copy; layout and structure unchanged.

### Code

- Created `code/` with 38 stage directories across `c/`, `cpp/` and `bio/`, each holding a README that names its chapter and lists the files actually present.
- Added 38 `labs/<track>/<stage>/check.sh` scripts, one per stage, each building a single target with the project flags and reporting warnings without failing on them.
- Added `code/CMakeLists.txt` with the `craft_common` interface target, the `craft_lab()` helper and `enable_testing()`.
- Added `code/include/craft_assert.h`, `craft_bench.h` and `craft_io.h`.
- Added `tools/Makefile` (configure, build, test, sanitize, labs, clean) and `tools/run_labs.sh`.
- Added `labs/c`, `labs/cpp`, `labs/bio` with their grading conventions, including the zero-warning rule for C++.
- Added `data/README.md` with accession numbers and download commands instead of committed coordinates.
- Added `kb/README.md` describing the local knowledge base used by the AI-assisted workflow chapter.

### Removed

- Removed `docs/assets/contentImag/`, 46 screenshots of the inherited template's administration panel.
- Removed `docs/setup/`, four pages describing template plugins, navigation and site settings.
- Removed `docs/getting-started/`, four pages describing template installation and configuration.

### Known gaps

The homepage demonstration image is still the inherited screenshot; it is kept as a placeholder, and the book's own terminal capture replaces it when one exists.

## 2026-11-21 — Licences added

### Documentation

- Added `docs/license.md`: the scope table, CC BY-NC-SA 4.0 and MIT in plain terms, the attribution format for text and for code, the route for commercial permission, the third-party components, and the two limits a licence cannot cross.
- Added `## 7. Licences: what you may do with this book` to `start-here/preface.md`, with the reason for splitting the licence and the boundary between prose and code; `Where to start` moved to `## 8.`.
- Set the site footer to `Copyright © 2026 HUIMY - Text CC BY-NC-SA 4.0, code MIT`, and added the licence page to the About section of the navigation.
- Added a `Licence` section to `README.md` and a third-party notice to `NOTICE`.

### Code

- Added `LICENSE` (MIT) at the repository root, covering `code/`, `labs/`, `tools/`, the build files and every code block inside a page.
- Added `SPDX-FileCopyrightText` and `SPDX-License-Identifier: MIT` headers to all 65 C and C++ translation units.

## 2026-11-21 — Editorial pass: numbering, lead-ins, depth

### Documentation

- Renumbered every content page's second-level headings as `## 1.`, `## 2.`, … in reading order; `## Continue` stays unnumbered because it is page navigation, not a section. The hierarchy is now H1 plus numbered H2 on all 60 pages.
- Added a first-person lead-in paragraph after the H1 of all 58 chapters, each written for its own page: it states the order of the argument and what was left out, rather than summarising the sections.
- Deepened twelve chapters where the explanation had been a definition followed by a table: `c/pointers.md` (tcache overwrites the first sixteen bytes of a freed chunk), `cpp/classes.md` (three sizes that contain the object model), `cpp/lifetime.md` (the three failures of the naive assignment), `cpp/inheritance.md` (the two parts of `delete` through a base pointer), `cpp/polymorphism.md` (the three steps of slicing), `cpp/operators.md` (cross-multiplication overflow at 2^62), `cpp/containers.md` (why 1.5× can reuse freed blocks and 2× cannot), `cpp/modern.md` (`auto` deduction and the `vector<bool>` proxy), `cpp/exceptions.md` (`move_if_noexcept` choosing copy at compile time), `cpp/algorithms.md` (the unguarded partition that `<=` walks past the end), `bio/alignment.md` (BLOSUM as half-bit log-odds and why scores add), `engineering/compile-pipeline.md` (`<optimized out>` and `-Og`).
- Rewrote `about/ai-assisted-workflow.md` in the first person around one real project: the local Obsidian knowledge base at `生物信息学工具/多肽对接/多肽生成器` with its 28 notes, the `2-8` numbering collision that fed a prompt the wrong note, three pytest contracts, and a 3.2M-row run killed with `kill -9` that still passed `PRAGMA integrity_check`.
- Rewrote `start-here/about-the-author.md` in the first person, keeping the same facts.
- `start-here/linux-toolchain.md` presents three routes as peers — Linux or WSL2 is what the book builds on, Visual Studio 2022 and conda-forge are first-class alternatives with their own commands — rather than treating Linux as the only option.

### Fixed

- Two lead-in paragraphs (`cpp/inheritance.md`, `cpp/templates.md`) were initially skipped because a forty-character prefix already occurred in the body; they were inserted with full-text matching.

### Known gaps

The seven labs whose assertions were corrected against real sanitizer output (`c_02_lifetime`, `c_05_strlen`, `c_15_list`, `cpp_10_algorithms`, `cpp_12_exceptions`, `bio_03_lse`, `bio_05_pdb`) have not been re-run since those edits, so `ctest` is expected to pass but has not been observed to. The homepage demonstration image is still the inherited screenshot; it is kept as a placeholder, and the book's own terminal capture replaces it when one exists.
