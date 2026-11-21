# C++ Reference

Six tables, each answering a question that comes up in review. Special member functions and when the compiler writes them, value categories in one table, overload resolution order, the containers and algorithms worth memorising, the C++20 flags, and the three rules that prevent most of the defects I have seen.

The C++ counterpart, organised by what a reader looks up rather than by what a chapter teaches.

## 1. The special member functions

| Function | Signature | Generated when |
| --- | --- | --- |
| default ctor | `T()` | no other ctor declared |
| copy ctor | `T(const T &)` | no move declared |
| copy assign | `T &operator=(const T &)` | no move declared |
| move ctor | `T(T &&) noexcept` | no copy, move or dtor declared |
| move assign | `T &operator=(T &&) noexcept` | same |
| destructor | `~T()` | always |

Declaring any one of the copy or move operations suppresses the others; declaring a destructor suppresses the moves. The rule of five: if you need one, you probably need all five, or you should need none because your members are already RAII types.

```cpp
struct Rule {              // the recommended default: none of the five
    std::vector<int> v;    // copyable, movable, destructible, all correct
};
```

## 2. Value categories, in one table

| Category | Example | Binds to |
| --- | --- | --- |
| lvalue | named object, `*p`, `a[i]` | `T &` |
| xvalue | `std::move(x)`, `T{}` member access | `T &&` |
| prvalue | `42`, `f()` returning by value | `T &&`, materialises to `T` |
| — | `const` lvalue | `const T &` only |

`std::move` is an unconditional cast to `T &&`; it does not move anything by itself. `std::forward<T>` preserves the value category and is what a forwarding constructor needs.

## 3. Overload resolution order

1. Exact match.
2. Promotion (`char` → `int`).
3. Standard conversion (`int` → `double`).
4. User-defined conversion (one, at most).
5. Ellipsis.

Two candidates reached at the same step make the call ambiguous. The book's rule: a function taking `double` and one taking `int64_t` makes every `int` call ambiguous, which is why overloads should differ in arity or in a way the caller can see.

## 4. Standard library, the containers and the algorithms

| Need | Use | Note |
| --- | --- | --- |
| dynamic array | `std::vector<T>` | default choice |
| fixed size array | `std::array<T, N>` | no allocation |
| text | `std::string` | owns |
| text parameter | `std::string_view` | borrows, must not outlive |
| key-value, ordered | `std::map` | O(log n), stable iterators |
| key-value, unordered | `std::unordered_map` | O(1) average, order unspecified |
| maybe a value | `std::optional<T>` | explicit absence |
| exclusive owner | `std::unique_ptr<T>` | zero overhead |
| shared owner | `std::shared_ptr<T>` | atomic count, cycles leak |
| observer | `std::weak_ptr<T>` | `lock()` to use |

| Need | Algorithm |
| --- | --- |
| sort | `std::sort`, `std::stable_sort` |
| select k-th | `std::nth_element` |
| find | `std::find`, `std::find_if` |
| transform | `std::transform` |
| remove | `std::remove_if` + `erase` |
| accumulate | `std::accumulate` (numeric) |
| min / max | `std::min_element`, `std::max_element` |
| binary search | `std::lower_bound`, `std::upper_bound` |

## 5. Compiler flags for C++20

| Flag | Effect |
| --- | --- |
| `-std=c++20` | the standard (MSVC: `/std:c++20`) |
| `-Wall -Wextra` | the useful warnings |
| `-Wconversion` | narrowing conversions |
| `-Wshadow` | a local shadows an outer name |
| `-Wnon-virtual-dtor` | a polymorphic class with a non-virtual dtor |
| `-Wold-style-cast` | C-style casts in C++ |
| `-Werror` | warnings become errors (the chapter rule is zero warnings) |
| `-g` | debug info for the sanitizer reports |
| `-fsanitize=address,undefined` | ASan + UBSan |
| `-D_GLIBCXX_DEBUG` | libstdc++ container checks |
| `-D_GLIBCXX_ASSERTIONS` | cheaper subset of the above |

`-Wnon-virtual-dtor` is the flag that catches deleting a derived object through a base pointer without a virtual destructor — the defect that compiles, runs, and leaks.

## 6. The three rules that prevent most C++ defects

1. **Own with RAII.** A raw `new`, `malloc`, `fopen`, or `socket()` in a constructor-less context is a leak waiting for the second exit path.
2. **Pass by reference for input, by value only for cheap copies.** A `const T &` parameter costs nothing and accepts everything; a `T` parameter copies.
3. **`noexcept` on move operations.** Without it, containers copy instead of move, and the performance claim of move semantics does not hold.

## Continue

- [C Reference](c.md) is the C side of the same tables.
- [Compiler Flags](compiler-flags.md) is the flag reference with the platform differences.
- [Error Cookbook](error-cookbook.md) is the message-to-cause table.
