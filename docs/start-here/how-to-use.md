# How to Use This Book

This page is the map, and I keep it concrete: what lives where, how the numbering works, two ways to run an example, the lab protocol, how to use a model without outsourcing the thinking, three routes through the book, and what done means. If you are unsure whether to read or to type, the answer is here.

Three things determine whether this book works for you: how the examples are numbered, how the exercises are graded, and what you do when you are stuck. Each is mechanical, and each is worth ten minutes before you write any code.

## 1. What lives where

The repository has five directories, and each has exactly one job.

| Path | Holds | You touch it when |
| --- | --- | --- |
| `docs/` | This book | Reading |
| `code/` | Every example, grouped by chapter number | Following a page, or copying a snippet to modify |
| `labs/` | One `check.sh` per chapter, recording that the lab still runs on your machine | Practising |
| `data/` | Download instructions for public structures and sequences | Starting the biology section |
| `kb/` | A local knowledge base, with notes you write yourself | When a question keeps recurring |

Nothing in `docs/` is generated from `code/` or the reverse. The page and the source are written together and reviewed against each other, which is why a page never claims behaviour that the source does not exhibit.

## 2. The numbering

Every example carries a label of the form `LAB 07-02`, meaning chapter 7, example 2. The chapter number is also the numeric prefix of the stage directory, so the label resolves to a path without a lookup table:

```
LAB 07-01  →  code/c/07_heap/01_growth_trace.c
             docs/c/heap.md  (chapter 7)
```

The same numbering is used in `labs/`: `labs/c/07_heap/` belongs to the same chapter. A page, a source file and an exercise that share a number are guaranteed to be about the same subject.

Each source file begins with a header comment that answers three questions before the reader compiles anything:

```c
/* LAB 07-01  heap/01_growth_trace.c
 * Drills: realloc growth, the address that changes, and the capacity policy.
 * Build:  cmake --build build --target c_07_growth
 *
 * The trace is the point: the addresses are the evidence that realloc moves
 * the block, which is why every pointer into the old block becomes invalid.
 */
```

`LAB` is the label the prose uses and the path from the repository root. `Drills` is what the program is evidence for, not what it contains. `Build` is the exact one-line command that produces the binary; the CMake target name is the one `labs/` uses too. Anything after the blank line is the sentence the chapter would otherwise have to repeat. When a page and a header disagree, the header is authoritative: it was compiled.

## 3. Two ways to run an example

Build everything at once through CMake, from the repository root:

```bash
cmake -S code -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

`ctest` runs each example as a test, so a chapter is "done" when the suite is green. This is also the fastest way to check that your toolchain matches the one the book assumes.

Build a single file when you are studying one page and do not want to wait for the whole tree:

```bash
cd code/c/07_heap
gcc -std=c17 -Wall -Wextra -Wpedantic -I../../include \
    01_growth_trace.c -o /tmp/growth
/tmp/growth
```

Compile to `/tmp` rather than into the source directory. Binaries in the source tree end up in `git status`, and a stale binary is a reliable way to spend an evening debugging code you already fixed.

## 4. The lab protocol

Each lab directory holds one file, `check.sh`, and the task itself lives in `code/<track>/<stage>/README.md` plus the Lab section of the chapter. There is no starter file by design: the point of the exercise is to write the program, not to fill three blanks in someone else's. The protocol is deliberately rigid, because the value of an exercise is in the prediction, not the answer:

1. Read the chapter's Lab section and the stage `README.md`, and write down, on paper, what you expect the program to print.
2. Write the program into `/tmp/lab` from memory, with the chapter closed. Do not read `code/` yet.
3. Compile it with the flags from the chapter and run it. Compare the output with your note, not with the book.
4. Only when it differs, open the chapter section, then `code/`. A difference you predicted is a gap in your model; one you did not predict is a gap in your reading.
5. Run `bash check.sh` in the lab directory to confirm the reference implementation still produces the expected result on your machine — a toolchain regression, not your answer, is what it rules out.
6. Change one thing, rerun. Two changes at once makes the next failure uninterpretable.

Run every lab at once from the repository root:

```bash
bash tools/run_labs.sh
```

The script prints one line per lab and exits non-zero if any failed. A build that emits a warning prints the warning text and continues, so the zero-warning rule stays visible without turning a known diagnostic into a false failure.

## 5. Using a model without outsourcing the thinking

This book contains prompts at the points where readers most often get stuck, marked `Prompt the model`. They are meant to be used after you have formed your own guess, not before. Three rules make that work:

**Read the diagnostic first, then ask.** A compiler message contains the file, the line, the rule name, and often a note explaining the fix. Handing the message to a model without reading it transfers the work you were supposed to do. Read it, form a hypothesis, then ask whether the hypothesis is right.

**Demand evidence, not confidence.** A useful answer cites a standard clause, a compiler version, or a program that can be compiled and run. An answer that says "in most implementations" without naming one is a guess wearing a lab coat. Ask for the code, then compile it.

**Treat the answer as a hypothesis.** The compiler and the sanitizer are the arbiters, not the model. When the two disagree, the compiler is right, and the disagreement itself is worth understanding — it usually means the model learned a rule from a different standard version or a different compiler.

The full method, including how to build the local knowledge base that grounds these questions, is in [How I Use AI to Build a Project](../about/ai-assisted-workflow.md).

## 6. Three routes through the book

Not every reader needs all sixty pages in order. Three routes cover the common cases.

| Route | For | Path | Time |
| --- | --- | --- | --- |
| **Inspection** | Readers who already write C and want the object model | Preface → Objects in Memory → Stack Frames → Pointers → Structs and Padding → Undefined Behavior → RAII → PDB Parsing | 2–3 weeks part-time |
| **Complete** | Readers learning both languages properly | Everything, in navigation order, with every lab attempted | One semester |
| **Domain-first** | Biologists who need working code, not a curriculum | Preface → First Program → Strings → Files → Bit-Level Programming → the whole Computational Biology section, consulting the C chapters on demand | 4–6 weeks |

The domain-first route works because the biology section assumes only the C chapters it links back to. Each page in it names its prerequisites at the top and repeats the two or three facts it borrows, so a reader arriving from a different direction does not have to reconstruct the whole earlier section.

## 7. What "done" means

A chapter is finished when three things hold: the example builds with no warnings, its `Expect` line matches what you observed, and you can explain the `Probe` result without rereading the page. If the third condition fails, the chapter is not finished even though the code runs; that is the difference between having executed a program and having learned from it.

## Continue

- [Preface](preface.md) explains why the book is ordered this way, if you skipped it.
- [Linux Toolchain](linux-toolchain.md) covers installation, including the sanitizer runtime check.
- [Objects in Memory](../c/objects-in-memory.md) is the first chapter, and the one the rest of the C section builds on.
