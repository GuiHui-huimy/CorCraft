# Security

This is a book repository, not a service, so the threat model is not the usual web one. The risk here is different and more practical: a reader compiles and runs code that was written to fail, on a machine that holds their own work.

## 1. Why a security page belongs in a programming book

Several examples in this book contain defects on purpose. A chapter on the heap demonstrates a double free. A chapter on arrays writes one element past the end. A chapter on undefined behaviour relies on the compiler doing something unspecified. Each of these is correct pedagogy and poor operational advice if the resulting binary is run outside the context that produced it.

The distinction is worth stating plainly: the *code* is deliberately broken, the *repository* is not. Nothing in `code/` is intended to be linked into another program, installed system-wide, or run against data you cannot afford to lose.

## 2. Running the examples

Four rules apply to every lab in this repository.

**Build into `build/` or `/tmp`, never into the source tree.** A binary in the working directory is easy to run by accident and easy to commit by mistake. The header comment in each file states the output path; use it.

**Sanitize first, then trust.** Memory chapters run under `-fsanitize=address,undefined`. A lab that "works" without the sanitizer may still be writing out of bounds; the sanitizer is what makes the difference visible. This is also why the C++ labs fail on warnings: a warning is the compiler telling you it already suspects something.

**Do not elevate.** None of the commands in this book requires `sudo`. If a step asks for root, the step is wrong, or the environment is. Installing a compiler is the single exception, and it is confined to the toolchain page.

**Do not run binaries you did not build.** A prebuilt executable in an issue or a pull request is not part of the book. Build from source or do not run it.

Containers are a reasonable belt for the chapters on undefined behaviour, since a container limits what a corrupted write can reach. They do not make undefined behaviour defined; they only confine the damage.

## 3. Input handling

The parsing labs read files written by other people, which is the situation where a parser is most likely to be attacked in the wild. Three habits are taught explicitly and are expected in contributed code:

- **Validate width before indexing.** A PDB line is 80 columns. A line of 60 columns is not truncated data to be padded; it is a malformed record to be rejected.
- **Bound the input size.** `craft_read_file` grows a buffer but the caller still checks how large the result is. A 4 GB file is not a test case, it is a denial of service.
- **Never use `gets`.** It cannot be made safe, because it has no length parameter. `fgets` with an explicit size, and a check that the newline was actually consumed, is the minimum.

A parser that skips a malformed line silently is worse than one that stops: the first produces a structure that looks complete and is not.

## 4. Data

All structures used in the biology section are public: PDB entries from RCSB, sequences from UniProt. Nothing else should ever enter `data/`.

Two things are not committed, ever: coordinates of unpublished structures, and file paths that reveal a private filesystem layout. The second is subtle — an example that prints `/home/<name>/projects/target_x/...` publishes more about its author than about C++.

## 5. Reporting a problem

Security-relevant reports go to `suguohui_account@163.com` privately, not into a public issue. That covers a lab that can damage a host outside its container, an instruction that requires privileges it should not, and a page that recommends an unsafe pattern without marking it as a counter-example.

Reports about a deliberately broken example are welcome but are not vulnerabilities: if the page says the code is wrong, the code being wrong is the point.
