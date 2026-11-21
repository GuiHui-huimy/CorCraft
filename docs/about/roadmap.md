# Roadmap

A roadmap is only useful if it can say no, so this page has a section for what is written and tested, one for what is in progress, one for what I am deliberately not planning, and one for the decisions I have not made. I date the entries, because an undated roadmap is a wish.

What is written, what is not, and what will not be. Stated plainly because a reader deserves to know which pages are finished and which are sketches.

## 1. Written and tested

| Section | Pages | State |
| --- | --- | --- |
| Start Here | 5 | written; toolchain verified on Route A |
| C | 16 | written; all chapters have a Field Note and a lab |
| C++ | 13 | written; each has a Practise section |
| Engineering | 5 | written; flags table differs between GCC and MSVC |
| Biology | 10 | written; the three structures are used throughout |
| Reference | 6 | written; the error table is from actual messages |
| About | 3 | written |

That is 58 chapters plus the home page. The number is the plan's, and no page is a placeholder: every page that exists has its argument, its measurement or its quote.

## 2. In progress

- **Code.** All 38 stage directories hold their sources: 37 files under `code/c/`, 14 under `code/cpp/`, 13 under `code/bio/`, plus three shared headers — 64 numbered targets and one for the first page's `warn.c`, for 65 `ctest` entries. Every `LAB` reference in the prose resolves to a file.
- **Labs.** Each of the 38 stages has its `check.sh` beside the chapter's directory, and the three track READMEs state the protocol. There is no starter file by design; the exercise is the rewrite described in each chapter's Practise or Lab section.
- **Verification.** Seven assertions were corrected after they failed against real sanitizer output — two in the C chapters, one in the C++ algorithms and exceptions labs, and two in the biology numerics and PDB labs. Those edits have not been re-run under `ctest`, so the suite is expected to pass rather than observed to.

## 3. Deliberately not planned

| Thing | Why not |
| --- | --- |
| A BLAST chapter | running an existing aligner is a shell exercise; the algorithm is chapter 04 |
| A machine-learning framework chapter | a different book; the closest is the numerics of a scoring model |
| A database chapter | querying UniProt is a network and schema exercise |
| A Python chapter | the book is C and C++; Python appears only as a comparison in the AI workflow |
| Vendored structure files | `data/README.md` gives the fetch commands instead |
| A theme fork | the Material theme is a dependency; `docs/material/overrides` holds only the home page |

## 4. Open decisions

- **License.** The repository has no license file and the pages do not claim one. This is a decision for the owner and will be made before the first public release.
- **Home page image.** The template's demo screenshot is still in place. It will be replaced with an image from this book's own build, generated rather than captured, when the documentation work is complete.
- **Route B coverage.** The MSVC sections are written but not yet exercised on a Windows machine for every lab. UBSan has no MSVC equivalent, and the chapters say so; the remaining work is confirming that each lab that mentions a Route B difference actually builds.

## 5. How to follow the work

The changelog records each batch with what was added and what was removed. A reader who wants to know whether a page is current should read that first; a reader who wants to know what to expect should read the section table above.

## Continue

- [Changelog](../changelog.md) is the dated record.
- [Contributing](../contributing.md) is the workflow for adding a lab.
- [Repository Layout](../reference/repo-layout.md) is the tree these batches modify.
