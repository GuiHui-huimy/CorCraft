# Linux Toolchain

Three routes, one compiler underneath, and I say so before the instructions: Linux or WSL2 is what this book uses, Visual Studio 2022 is a first-class alternative on Windows, and conda-forge gives you GCC or Clang without touching the system. I give commands for all three, then one verification program that has to pass on any of them.

Three routes lead to a usable C and C++ compiler, and all three run every example in this book. The difference is what the book's commands assume: this text is written against Linux, because a compiler that reports identical warnings on every machine is what makes the examples reproducible, and because the clusters that run the biology code are Linux machines.

That is a choice of reference implementation, not a claim about the only correct environment. Visual Studio 2022 on Windows is a complete, first-class toolchain, and a conda-forge environment gives you a compiler that travels with your analysis code. Sections 3 and 4 describe both, including the few places where a chapter needs a note.

## 1. Three routes at a glance

| Route | Best for | Compiler | What changes in the book |
| --- | --- | --- | --- |
| **A. Linux or WSL2** | The reference route; all fifty-eight chapters | GCC 11+ or Clang 14+ | Nothing |
| **B. Visual Studio 2022** | Windows-native work, MSVC debugger, C++20 | MSVC 19.3x | Compiler flags differ; `perf` and a few POSIX calls are replaced by chapter notes |
| **C. conda-forge** | Readers already living in conda; reproducible scientific environments | GCC or Clang from conda | Toolchain paths are long; set `CC`/`CXX` explicitly |

Whichever route you take, the language standards are fixed: C17 and C++20. A compiler that cannot accept `-std=c17` and `-std=c++20` will fail on the first C++ chapter, and it is worth checking that before reading further.

## 2. Route A — Linux or WSL2

Any of the following is sufficient:

| Environment | Notes |
| --- | --- |
| Ubuntu 22.04 or 24.04 LTS | The reference platform; every command in this book was written against it |
| Debian 12 | Same packages, same paths |
| WSL2 with Ubuntu 22.04 | Identical inside the distribution; keep project files on the Linux filesystem |
| A container based on `ubuntu:22.04` | Fine for the build chapters, awkward for the debugger chapters |

Two version floors matter. CMake must be at least 3.20, because the top-level `CMakeLists.txt` declares `cmake_minimum_required(VERSION 3.20)`. GCC must be at least 11 for the C++20 chapters; GCC 9 covers everything in the C sections. Ubuntu 20.04 ships CMake 3.16 and is therefore below the floor — install a newer copy from Kitware's repository rather than fighting the system package.

Install in one command:

```bash
sudo apt update
sudo apt install -y build-essential clang gdb cmake valgrind git curl
```

`build-essential` pulls in `gcc`, `g++`, `make` and the C library headers. `clang` is installed alongside GCC, not instead of it: the two compilers disagree on some diagnostics, and reading both opinions on the same file is one of the fastest ways to learn what a warning means. `valgrind` covers the one chapter where AddressSanitizer cannot see the problem.

On WSL2, install the distribution first, then run the same commands inside it:

```powershell
wsl --install -d Ubuntu-22.04
```

Keep the repository inside the Linux home directory — `/home/<user>/craft` — and not under `/mnt/c/...`. Cross-filesystem access through the WSL mount is an order of magnitude slower, which makes the performance chapter meaningless and the build chapters tedious.

## 3. Route B — Visual Studio 2022

Visual Studio 2022 with the **Desktop development with C++** workload provides the MSVC compiler, a debugger, CMake integration and AddressSanitizer. Nothing in the book's object model depends on GCC, so the C and C++ chapters work unchanged once the flags are translated.

Install the workload through the Visual Studio Installer: open **Modify**, tick **Desktop development with C++**, and confirm that *MSVC v143*, *Windows SDK* and *C++ CMake tools* are selected. About 3 GB.

Build from the **Developer Command Prompt for VS 2022**, not from an ordinary terminal; the prompt sets `PATH`, `INCLUDE` and `LIB`:

```bat
cl /std:c17 /W4 /WX /Zi hello.c
cl /std:c++20 /W4 /WX /EHsc /Zi hello.cpp
```

The mapping from the book's flags is direct but not identical:

| Book | MSVC | Note |
| --- | --- | --- |
| `-Wall -Wextra -Wpedantic` | `/W4` | MSVC's `/W4` is roughly `-Wall -Wextra`; there is no pedantic tier |
| `-Werror` | `/WX` | Any warning becomes an error |
| `-g` | `/Zi` | Debug information, used by the VS debugger |
| `-fsanitize=address` | `/fsanitize=address` | Supported since VS 2019 16.9; needs the ASan runtime DLL on `PATH` |
| `-std=c17` | `/std:c17` | Supported since VS 2019 16.7 |
| `-std=c++20` | `/std:c++20` | Supported since VS 2019 16.11 |
| `-O2` | `/O2` | Same intent, different inlining thresholds |

The CMake project builds with the Visual Studio generator:

```bat
cmake -S code -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config RelWithDebInfo
ctest --test-dir build -C RelWithDebInfo --output-on-failure
```

Three chapters need a substitution note, and each carries one on the page itself: `engineering/performance.md` uses `perf stat`, which has no MSVC equivalent (use the VS profiler or `Measure-Command`); `bio/parallel-scoring.md` uses OpenMP, which MSVC supports through `/openmp` with a different runtime; and `c/files-and-streams.md` uses `fopen` only, so it is unaffected, but the `errno` values differ in wording.

Two things to expect. First, MSVC's diagnostics are worded differently from GCC's, so the verbatim warning text quoted in the field notes will not match your screen; the flag or the rule name is what to compare, not the sentence. Second, MSVC does not implement every C99 variable-length array case and rejects some forms GCC accepts; where a lab relies on one, the page says so.

## 4. Route C — conda-forge

A conda environment is a reasonable choice when the compiler has to travel with an analysis stack: the same environment then holds the compiler, CMake, a Python interpreter and the scientific packages, and it can be reproduced from a file.

```bash
conda create -n craft -c conda-forge cmake make gdb
conda activate craft
```

The compiler packages are platform-specific, and the names change across platforms, so confirm rather than guess:

```bash
conda search -c conda-forge 'gcc*'
```

On Linux the packages are `gcc_linux-64` and `gxx_linux-64` (or the newer `gcc`/`gxx` metapackages); on macOS they are `clang_osx-64` and `clangxx_osx-64`; on Windows the practical option is `m2w64-toolchain`, a MinGW-w64 port. Install the pair that matches your platform:

```bash
conda install -c conda-forge gcc_linux-64 gxx_linux-64      # Linux
conda install -c conda-forge m2w64-toolchain                # Windows
```

Conda compilers are installed with a target triplet in the filename, so the generic `gcc` command may not exist. List what actually landed and point CMake at it:

```bash
ls "$CONDA_PREFIX/bin" | grep -E 'gcc|g\+\+'
export CC="$CONDA_PREFIX/bin/x86_64-conda-linux-gnu-gcc"
export CXX="$CONDA_PREFIX/bin/x86_64-conda-linux-gnu-g++"
```

The exact triplet varies by platform; the two lines above are for `linux-64`. On macOS the prefix is `x86_64-apple-darwin` or `arm64-apple-darwin`, and on Windows the binaries are named `x86_64-w64-mingw32-gcc`. Setting `CC` and `CXX` before invoking CMake is what makes `cmake -S code -B build` use the conda compiler rather than the system one:

```bash
CC="$CC" CXX="$CXX" cmake -S code -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

Two caveats. Sanitizers are not shipped with every conda GCC build; check with `gcc -fsanitize=address -x c - -o /tmp/probe <<< 'int main(void){return 0;}'` before relying on them. And `gdb` from conda sometimes cannot read debug info produced by a different compiler build; if that happens, use the system `gdb`.

## 5. Verify whichever route you chose

The checks are the same idea everywhere: prove that C17 and C++20 are accepted, and prove that the memory checker works.

On Linux, WSL and conda:

```bash
gcc --version | head -n1
clang --version | head -n1        # optional
cmake --version | head -n1

printf 'int main(void){return 0;}\n' > /tmp/probe.c
gcc -std=c17 -Wall -Wextra -Wpedantic /tmp/probe.c -o /tmp/probe && /tmp/probe
g++ -std=c++20 -Wall -Wextra -Wpedantic -x c++ /tmp/probe.c -o /tmp/probe && /tmp/probe
gcc -fsanitize=address,undefined -g /tmp/probe.c -o /tmp/probe_san && /tmp/probe_san
echo $?
```

On Windows with MSVC:

```bat
cl /std:c17 /W4 /WX /Zi probe.c
cl /std:c++20 /W4 /WX /EHsc /Zi probe.cpp
cl /std:c17 /fsanitize=address /Zi probe.c
```

Expected results: no diagnostics, exit status `0` everywhere, and an ASan run that terminates normally. A loader error mentioning `libasan` on Linux means the sanitizer runtime package is missing (`libasan5`, or `libasan8` on newer releases); on Windows it means the ASan runtime DLL is not on `PATH`.

The version floors, once more, as a table to check against:

| Tool | Floor | Reason |
| --- | --- | --- |
| GCC | 11 | C++20 chapters |
| Clang | 14 | Comparable C++20 support |
| MSVC | 19.3x (VS 2022) | `/std:c++20` and ASan |
| CMake | 3.20 | `CMakeLists.txt` floor |
| GDB | 9 | Pretty-printing and `-O1` debugging |

## 6. Workspace layout

All three routes use the same directory shape: the repository, the build tree beside it, and the downloaded data.

```bash
mkdir -p ~/craft && cd ~/craft
git clone https://github.com/GuiHui-huimy/CorCraft.git
cd CorCraft
cmake -S code -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
```

The build tree lives outside the source tree so that `git status` stays clean and a broken build can be deleted with a single `rm -rf build`. Nothing in the repository writes into the source tree; the labs write their binaries into `build/` or `/tmp`.

The layout is described in [Repository Layout](../reference/repo-layout.md). For now two paths matter: `code/` holds the sources and the CMake project, and `labs/` holds the exercises.

## 7. When the install goes wrong

**`apt` cannot find `clang`.** The package index is stale, or the sources list points at a release that does not carry it. Run `sudo apt update`; if that fails, compare `/etc/apt/sources.list` with `lsb_release -cs`.

**CMake reports a version below 3.20.** Either install a newer release from Kitware, or keep an isolated copy: `python3 -m pip install --user cmake`. The second route is the one to prefer on a shared machine.

**GCC accepts `-std=c17` but rejects `-std=c++20`.** GCC 9 and 10 accept the flag and implement only part of the standard. Install `g++-11` and select it with `CXX=g++-11`.

**AddressSanitizer reports a shadow-memory conflict.** Some sandboxed containers reserve the address space ASan needs. Run the same binary outside the sandbox, or use Valgrind for that chapter; the diagnostic content is equivalent, only slower.

**MSVC cannot find `cl`.** An ordinary terminal has no `INCLUDE` or `LIB` paths. Open the Developer Command Prompt, or run `vcvars64.bat` first.

**Conda's `gcc` is not found after activation.** The binary carries a triplet prefix. List `$CONDA_PREFIX/bin` and set `CC` and `CXX` as in section 4.

## Continue

With a compiler verified, the next step is to compile something and read what it says: [First Program](first-program.md). If you would rather see how the exercises are structured before starting, read [How to Use This Book](how-to-use.md). The per-flag comparison table lives in [Compiler and Flags](../reference/compiler-flags.md).
