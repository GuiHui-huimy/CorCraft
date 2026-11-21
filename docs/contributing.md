# Contributing

This repository holds a book and the code that the book describes. A contribution can therefore take two forms, and the two are reviewed by different standards: a page is judged by whether its claims can be verified, a source file by whether it compiles clean.

## 1. What is worth sending

Four kinds of contribution are accepted without discussion:

- **A reproducible defect.** A page states a fact — a `sizeof` value, a warning text, an exit status — and the reader's compiler disagrees. Send the compiler version, the command, and the output. If the page is wrong, it is fixed; if the page is right and the compiler differs, the page gains a note about the version.
- **A lab that is ambiguous.** The `README.md` asks for something that two careful readers interpret differently. That is a documentation defect, not a reader defect.
- **A field note.** A defect from your own code, in the shape described in section 4. These are the most useful contributions because they carry a real compiler response.
- **A missing prerequisite link.** A page assumes a fact it never establishes and never links to.

Feature requests for the site design, the theme, or the build system are out of scope: the theme is inherited and deliberately left alone.

## 2. Rules for source files

Every example obeys five rules. A pull request that breaks one is returned with the rule cited, not with the fix applied.

1. **One translation unit.** The file compiles alone, with its own `main`. No example depends on another example's object file.
2. **Four-line header.** `LAB`, `Build`, `Expect`, `Probe`, in that order. `Build` is a command that works from the file's own directory; `Expect` includes the exit status; `Probe` names one change worth making.
3. **Zero warnings.** `-Wall -Wextra -Wpedantic` for C, the same for C++, plus `-Wconversion` where the chapter is about conversions. The build does not pass `-Werror`, because a reader who cannot run the program also cannot read the warning; zero warnings is the review rule, and `check.sh` prints what it sees so the rule stays visible.
4. **No writes into the source tree.** Output goes to `build/` or `/tmp`. A committed binary is a bug.
5. **Deterministic output.** Same input, same bytes out. Where randomness is part of the lesson, the seed is fixed and printed.

Dependencies are limited to the C and C++ standard libraries. A third-party library may appear only in a chapter whose subject is that library, and then it is optional.

## 3. Rules for pages

Pages are written in English and are judged on density rather than length. Five to nine kilobytes of prose, tables and code is the target; a page below that is usually missing evidence, and a page above it usually contains a paragraph that belongs on another page.

Three requirements apply to every page:

- **Every claim has evidence.** A number from a `sizeof`, a warning copied verbatim from a compiler, a timing from a stated machine, or an assertion that runs. A claim without evidence is removed, however plausible.
- **Links resolve to a subsection.** `[Heap](c/heap.md#realloc-may-move-the-block)` is acceptable; "see the next chapter" is not. A link into a page that does not exist is a build error, not a promise.
- **No repeated paragraph.** The same explanation does not appear on two pages. One page owns it; the other links to the anchor.

Structure is not templated. The memory chapters run as a continuous argument; the craft chapters open with a failure; the build chapters are organised around commands and artefacts; the biology chapters around data flow. A page is allowed to look unlike its neighbour. It is not allowed to look unlike itself halfway through.

## 4. Field note format

A field note records a defect from real code, and has five parts in a fixed order:

```
### The original code
<the snippet, with the file it came from>

### What the tool said
<compiler or runtime output, verbatim, including the flag name>

### Why it happened
<the classification: syntax, ownership, boundary, aliasing, lifetime>

### The smallest fix
<the corrected lines, and nothing else>

### Where the lesson transfers
<link to the chapter that generalises the rule>
```

The verbatim tool output is the part that makes a field note worth reading. Paraphrased warnings teach nothing, because the reader will meet the real text, not the paraphrase.

## 5. Workflow

```bash
git clone https://github.com/GuiHui-huimy/CorCraft.git
cd CorCraft
git switch -c page/heap-realloc-note
make -f tools/Makefile test
bash tools/run_labs.sh
```

Both commands must exit zero before a pull request is opened. A documentation-only change still runs them, because a page that contradicts a passing test is the failure mode this repository exists to avoid.

In the pull request, state which chapter is affected, what the reader could not do before, and which command demonstrates the difference. A diff without a reproduction is not reviewable.

## 6. Review standard

A change is accepted when three questions have answers: does it build clean, does the affected page still state only what the code exhibits, and can a reader who has not seen the change follow the result. The third question is the one that rejects most submissions, and the one worth asking before writing the second paragraph.

## 7. See also

- [Repository Layout](reference/repo-layout.md) for naming and directory rules.
- [Linux Toolchain](start-here/linux-toolchain.md) for the compiler versions the standards assume.
- [How I Use AI to Build a Project](about/ai-assisted-workflow.md) for using a model while drafting, without letting it invent the evidence.
