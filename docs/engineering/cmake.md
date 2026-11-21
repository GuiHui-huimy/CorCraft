# CMake and Build Systems

I build the build system before the labs, because a lab you cannot rebuild in ten seconds is a lab you will not repeat. The page reads the top-level file line by line, then registers one lab, then runs it, and only then compares Linux, Visual Studio 2022 and conda — three routes, one CMake logic underneath. The field note is a header I compiled twice and lost an afternoon to.

The repository builds with CMake because the same three-line command must work on Linux, in WSL2, under MSVC, and inside a conda environment. This page explains what this repository's `CMakeLists.txt` actually does, in the order a reader will meet it.

## 1. The shape of the tree

```text
code/
├── CMakeLists.txt          # the only entry point
├── c/CMakeLists.txt        # per-language registration
├── cpp/CMakeLists.txt
├── bio/CMakeLists.txt
├── include/                # shared headers: craft_assert.h, craft_bench.h, craft_io.h
└── c/07_heap/01_growth_trace.c
```

`cmake` is run against `code/`, never against `code/c/`. A single entry point means one build directory and one set of options, which is what makes the labs reproducible.

## 2. The top-level file, line by line

```cmake
cmake_minimum_required(VERSION 3.20)
project(craft LANGUAGES C CXX)
```

The version floor is 3.20 because `target_link_libraries` with an interface target and generator expressions needs it, and because 3.20 is what Ubuntu 22.04 ships. Declaring both languages up front means the C and C++ standards are set once.

```cmake
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

`EXTENSIONS OFF` means `-std=c17` rather than `-std=gnu17`. This matters more than it looks: `gnu17` accepts several GNU extensions that MSVC does not, so a lab that compiles under `gnu17` may fail on Route B. Turning extensions off keeps the code portable by construction.

```cmake
add_library(craft_common INTERFACE)
target_include_directories(craft_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(craft_common INTERFACE m)
```

`craft_common` is an interface library: it has no source files, it only carries usage requirements. Every lab links it, so every lab gets the shared headers without repeating the include path. The difference between `INTERFACE` and `PUBLIC` is that `INTERFACE` requirements apply only to consumers, never to the library itself — correct here because there is no library.

```cmake
function(craft_lab target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE craft_common)
    target_compile_options(${target} PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME ${target} COMMAND ${target})
endfunction()
```

One function, called once per lab. `add_test` in the same function is what makes `ctest` a complete test suite without a second declaration. The sanitizer flags are added only for GCC and Clang: MSVC has `/fsanitize=address` but does not support the combination with UBSan in the same way, and Route B's chapter notes say so.

`-Wconversion` is in the list because the C chapters are about type width, and it is the flag that reports a narrowing conversion at the point of the conversion rather than as a wrong result later. It is also the noisiest flag in the list, which is why chapter 2's examples are written to satisfy it deliberately.

## 3. Registering a lab

```cmake
craft_lab(01_sizeof_table c/01_objects_in_memory/01_sizeof_table.c)
```

The target name is the lab's identity in `ctest`, in the file tree, and in the book's `LAB` numbering. Keeping the three identical means the error message from `ctest` names a file the reader can open.

The reason for one line per lab rather than `file(GLOB_RECURSE ...)`:

- A glob is evaluated at configure time, so a new file requires re-running CMake and produces a build that silently omits it. Explicit registration fails loudly.
- A glob cannot express per-file options, and the sanitizer exception for MSVC is per-target.
- `git log` on `CMakeLists.txt` becomes a list of what was added, which is the closest thing to a changelog the code has.

## 4. Building and testing

```bash
cmake -S code -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`-S`/`-B` is the modern form and does not depend on the working directory. `--output-on-failure` is not optional: without it, `ctest` prints a pass/fail summary and hides the assertion message, which is the only useful part.

The `tools/Makefile` wraps the three commands:

```bash
make configure
make build
make test
```

It exists for readers who prefer `make`, and it calls `cmake` rather than replacing it. There is exactly one build description, and it is the `CMakeLists.txt`.

## 5. Route differences

| | Linux GCC/Clang | MSVC (Route B) | conda (Route C) |
| --- | --- | --- | --- |
| Configure | `cmake -S code -B build` | `cmake -S code -B build -G "Visual Studio 17 2022"` | `cmake -S code -B build -DCMAKE_C_COMPILER=$CC` |
| Build | `cmake --build build -j` | `cmake --build build --config Debug` | same as Linux |
| Test | `ctest --test-dir build` | `ctest --test-dir build -C Debug` | same as Linux |
| Sanitizers | automatic | automatic per-target check | automatic |

The `--config` argument appears because a multi-config generator puts the binaries in `build/Debug/` rather than `build/`. A reader who forgets it gets "test executable not found", which is a path problem rather than a code problem.

## 6. Field note: the header that was compiled twice

The archived C++ project listed both `person.cpp` and `main.cpp`, and the latter included `person.h` — correctly, a declaration. The build worked. Later a `person.cpp` was added to the list a second time through a wildcard, and the link failed with `multiple definition of Person::Person()`.

The cause was not the duplicate: it was that the course project's `person.h` contained the constructor body.

```cpp
#ifndef PERSON_H
#define PERSON_H
class Person {
public:
    Person(std::string n) : name(n) {}    // defined in a header
};
#endif
```

The include guard makes the definition appear once per translation unit, which is exactly once too many when two units include it. The guard solves multiple inclusion, not multiple definition — those are different problems, and the guard's name makes the difference easy to forget.

The repair is the rule the book follows everywhere: a header declares, a `.c` or `.cpp` defines. `inline` is the exception for functions that must live in a header, and it is the reason `craft_io.h`'s functions are `static inline`.

## Continue

- [Compilation Pipeline](compile-pipeline.md) is what the commands ultimately run.
- [Testing and Sanitizers](testing.md) explains what `ctest` executes.
- [Translation Units](../c/translation-units.md) is the rule the include guard does not solve.
