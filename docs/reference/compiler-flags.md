# Compiler Flags

Flags are grouped by what they change, not by compiler. Warnings and the standard first, then sanitizers, then linking, then the flags that change behaviour rather than diagnostics, then conda, because the third route has its own flag names and its own traps.

Three compilers, one table. The GCC column is the reference; MSVC is what Route B uses, and the differences are the reason a lab can build on Linux and fail on Windows.

## 1. Warning and standard flags

| Purpose | GCC / Clang | MSVC |
| --- | --- | --- |
| C standard | `-std=c17` | `/std:c17` |
| C++ standard | `-std=c++20` | `/std:c++20` |
| Warning level | `-Wall -Wextra -Wpedantic` | `/W4` |
| Warnings are errors | `-Werror` | `/WX` |
| Narrowing conversions | `-Wconversion` | part of `/W4`, no separate flag |
| Shadowing | `-Wshadow` | `/W4` (partial) |
| Non-virtual dtor | `-Wnon-virtual-dtor` | `/W4` (partial) |
| Old-style cast | `-Wold-style-cast` | — |
| Debug info | `-g` | `/Zi` |
| Optimisation | `-O0` `-O2` `-Os` | `/Od` `/O2` `/O1` |
| Standard conformance | (default) | `/permissive-` |
| Source charset | `-finput-charset=UTF-8` | `/utf-8` |
| Exceptions | (default) | `/EHsc` |
| Preprocessor only | `-E` | `/E` |
| Assembly output | `-S` | `/FA` |
| Object only | `-c` | `/c` |

`/permissive-` is not optional on MSVC: without it the compiler accepts constructs that are not standard C++, and a lab that builds only on Route B can hide a non-portable construct. `/utf-8` matters when the source contains non-ASCII characters, which the biology comments do.

The mismatch worth knowing: `-Wconversion` has no MSVC equivalent, so a narrowing conversion that GCC reports as a warning is `/W4`'s `C4244` — reported on a different line, with a different name, and sometimes not at all for template instantiations. A reader on Route B who sees no warning where the book says there is one should assume the warning exists and check with `-Wconversion` on Route A before concluding the code is clean.

## 2. Sanitizers

| Detector | GCC / Clang | MSVC |
| --- | --- | --- |
| AddressSanitizer | `-fsanitize=address` | `/fsanitize=address` |
| UndefinedBehaviour | `-fsanitize=undefined` | — (no equivalent) |
| LeakSanitizer | included in ASan on Linux | `/fsanitize=address` includes it on recent versions |
| ThreadSanitizer | `-fsanitize=thread` | — |
| Stack protector | `-fstack-protector-strong` | `/GS` (default) |
| Frame pointer | `-fno-omit-frame-pointer` | `/Oy-` |

UBSan has no MSVC equivalent, which is a real gap: the signed-overflow and shift checks in the C chapters are only available on Routes A and C. Route B readers get ASan's memory checks and the compiler's own `/W4` diagnostics, and the chapter notes say so explicitly rather than pretending the coverage is equal.

Combining ASan and UBSan is supported on GCC and Clang:

```bash
gcc -fsanitize=address,undefined -fno-omit-frame-pointer -g main.c -o app
```

The `-fno-omit-frame-pointer` is what makes the stack traces usable; without it, the report's frames are sometimes missing.

## 3. Linking

| Purpose | GCC / Clang | MSVC |
| --- | --- | --- |
| Link a library | `-lfoo` | `foo.lib` on the command line |
| Library search path | `-Lpath` | `/LIBPATH:path` |
| Static | `-static` or `libfoo.a` | `/MT` (static CRT) |
| OpenMP | `-fopenmp` | `/openmp` |
| Math library | `-lm` (last) | (linked by default) |
| pthread | `-pthread` | (built in) |

`-lm` must come after the object files that use it; the linker resolves left to right and a library before its user is ignored. This is the single most common link-order error.

## 4. The flags that change behaviour, not just diagnostics

| Flag | Effect | Why it matters |
| --- | --- | --- |
| `-O2` | aggressive optimisation | deletes unused computations; a benchmark at `-O0` is not a benchmark |
| `-fno-strict-aliasing` | relaxes aliasing rules | some legacy code needs it; the book's code does not |
| `-ffast-math` | relaxes IEEE semantics | changes results; never in the labs |
| `-fwrapv` | signed overflow wraps | turns UB into defined behaviour; hides defects |
| `-DNDEBUG` | removes `assert` | a check that disappears in release |
| `-fsigned-char` / `-funsigned-char` | plain `char` signedness | a file format that relies on it is not portable |

`-ffast-math` and `-fwrapv` are the two that most often turn a failing lab into a passing one, and both do it by removing the check rather than the defect. The labs do not use either.

## 5. Route C: conda

```bash
conda install -c conda-forge gxx_linux-64 cmake make
export CC=$(which x86_64-conda-linux-gnu-gcc)
export CXX=$(which x86_64-conda-linux-gnu-g++)
```

The compiler binary is prefixed, so the plain `gcc` on `PATH` may be the system one. Setting `CC` and `CXX` explicitly is what makes CMake use the conda toolchain; without it, CMake finds the system compiler and the sanitizer runtimes from the two toolchains conflict at link time.

The conda packages are `gxx_linux-64` on Linux and `vs2019_win-64` on Windows, and the compiler is generally one major version behind the system one. The C++20 requirement means checking `$CXX --version` before starting, not after a failed build.

## Continue

- [Compilation Pipeline](../engineering/compile-pipeline.md) is what these flags control.
- [CMake and Build Systems](../engineering/cmake.md) is how the flags are applied per target.
- [Error Cookbook](error-cookbook.md) is the message-to-cause table.
