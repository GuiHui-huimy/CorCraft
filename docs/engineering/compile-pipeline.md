# Compilation Pipeline

Four stages sound abstract until one of them explains an error you have already seen. I take preprocessing first because it is pure text, then the translation unit, then sections in the object file, then linking as symbol resolution and relocation. Static against dynamic, then optimisation, then a 24 KB binary for a four-line program — by the end each stage has a size you can point at.

One `gcc main.c` hides four programs and seven file extensions. This page opens them, because every warning and every link error in this book comes from one of those stages, and knowing which stage produced it is most of the diagnosis.

## 1. The four stages

```text
main.c ──► [cpp] ──► main.i ──► [cc1] ──► main.s ──► [as] ──► main.o ──► [ld] ──► a.out
           preprocess          compile            assemble          link
```

| Stage | Program | Input | Output | What it does |
| --- | --- | --- | --- | --- |
| Preprocess | `cpp` | `.c` `.h` | `.i` | includes, macros, `#if`, comments stripped |
| Compile | `cc1` / `cc1plus` | `.i` | `.s` | parse, type-check, optimise, emit assembly |
| Assemble | `as` | `.s` | `.o` | assembly to machine code and a symbol table |
| Link | `ld` | `.o` `.a` `.so` | executable | resolve symbols, relocate, lay out segments |

The driver runs all four and deletes the intermediates. Each stage can be stopped and inspected:

```bash
gcc -E main.c -o main.i        # preprocess only
gcc -S main.c -o main.s        # stop before assembling
gcc -c main.c -o main.o        # stop before linking
```

## 2. Preprocessing is textual, and that is the whole problem

`#include` is a file insertion, not an import. `#define` is a token replacement, not a constant. The consequences are visible in the first chapter's examples: a macro without parentheses around its parameters expands wrongly, and an include that is missing a header produces a "unknown type" error in the file that included it, with no mention of the real culprit.

```bash
gcc -E -P main.c | head -40      # -P drops line markers
```

Reading the preprocessed output is the fastest way to answer "why does the compiler think this macro is that?" — the expansion is right there, one line, already wrong.

The other use is size: a translation unit that includes `<stdio.h>`, `<stdlib.h>` and `<string.h>` is several thousand lines after preprocessing, which is why compile times are dominated by headers rather than by the source file. This is the reason for the forward-declaration habit in chapter 13: an include that could have been a declaration costs every includer the parse time.

## 3. The compiler sees one translation unit at a time

`cc1` never sees `other.c`. It type-checks the `.i` file in isolation, and any name it cannot resolve becomes an *external* symbol to be resolved later. That is why:

- A function called without a prototype is assumed to take `int` (C89) or rejected (C23), and never checked against the definition in another file.
- A `static` function with no caller produces a "defined but not used" warning — the compiler knows it is invisible outside this unit.
- A missing `#include` produces an error in this unit even though the definition exists elsewhere.

```bash
gcc -c main.c -o main.o
nm main.o
```

`nm` lists the symbol table: `T` for a defined text symbol, `U` for undefined, `t` for a local one. A `U main` in your `.o` and a `T main` nowhere is the classic "undefined reference to main" in reverse; a `U craft_read_file` with no `T` anywhere is the classic missing object file.

## 4. Assembling produces sections, not just code

```bash
gcc -c main.c -o main.o
size main.o
readelf -S main.o
```

The object file has `.text` (code), `.data` (initialised globals), `.bss` (zero-initialised globals), `.rodata` (constants and string literals), and `.comment`. Two consequences worth knowing:

- A global initialised to zero costs nothing on disk (`.bss` is not stored), which is why `static int table[1024];` is free and `static int table[1024] = {1};` is 4 KB of `.data`.
- A string literal lives in `.rodata`, so writing to it is a segfault. This is the mechanism behind the `char *s = "abc"; s[0] = 'x';` crash in the strings chapter.

## 5. Linking resolves and relocates

```bash
gcc main.o util.o -o app
```

`ld` reads the symbol table of every input, matches each `U` against a `T`, and rewrites the addresses in the code. Three errors come from here and nowhere else:

| Message | Meaning |
| --- | --- |
| `undefined reference to 'foo'` | a `U` with no matching `T` in any input |
| `multiple definition of 'foo'` | a `T` in two objects — usually a function defined in a header |
| `relocation truncated to fit` | a code model problem, rare and always about a huge binary |

The second one is the reason headers declare and `.c` files define. Chapter 13 covers the translation-unit rules that produce it.

## 6. Static and dynamic linking

```bash
gcc main.c -o app              # dynamic: libc resolved at run time
gcc main.c -static -o app      # static: libc copied into the binary
ldd app                        # list dynamic dependencies
```

A dynamic executable is small and shares libc with the system; a static one is portable and independent, at the cost of size and of never receiving libc security updates. `ldd` on a binary you did not build is a security question, not a size question.

The book builds dynamically, because the sanitizers require it and because a 40 KB binary is easier to read with `nm`.

## 7. Optimisation changes what you can debug

```bash
gcc -O0 -g main.c -o app        # debug build: every variable has an address
gcc -O2 main.c -o app           # release build: variables are registers or gone
```

At `-O2` the compiler may reorder statements, delete variables, and inline functions, so a debugger shows values that no longer exist and a breakpoint lands on a line that is not the one you expected. The book builds every lab at `-O0 -g` and measures timing at `-O2`, and the two are separate builds for exactly this reason.

The first symptom a reader meets is `<optimized out>`. It is not a debugger failure: the variable was placed in a register, that register was reused by later code, and the DWARF location description has no answer for the current instruction pointer. `info locals` lists the variables that still have a location; the ones missing are gone, not hidden. `-Og` is the middle setting — it keeps the variables and line table a debugger needs while still doing inlining and a little scheduling — which is why I use `-O0 -g` for the labs and `-Og -g` when a defect only appears with optimisation enabled.

## 8. Field note: the binary that was 24 KB for a four-line program

The first chapter's `warn.c` prints a diagnostic and exits. Built with `gcc warn.c`, the binary is about 24 KB — and the source is four lines. Almost all of it is the startup code, the dynamic loader stubs, and the section alignment that the linker inserts by default.

Knowing this matters for two reasons. It stops the intuition that "small program means small binary" from becoming a diagnostic rule that misleads; and it explains why `size` reports a `.text` far larger than the code you wrote. `gcc -Os -s warn.c` gets the file under 16 KB, mostly by dropping symbols.

The lesson: the ratio between source and binary is not a measure of anything, and a binary's size is a property of the link, not of the code.

## Continue

- [CMake and Build Systems](cmake.md) is how these commands get organised.
- [Translation Units](../c/translation-units.md) is the C-side rules that the linker enforces.
- [Compiler Flags](../reference/compiler-flags.md) is the flag reference behind this page.
