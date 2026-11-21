# Modern C++

The modern features are not a style; they are a set of defaults that remove a class of bug each. I go through the ones I actually use — `auto`, structured bindings, `if` with initialiser, `string_view`, `optional`, `[[nodiscard]]`, `constexpr` — and for each I name the bug it removes. The last section lists what these features did not fix, because the point of the chapter is judgement rather than vocabulary.

This page collects the features that change how code is written rather than what it computes. Each one is here because it removes a defect class, not because it is new.

## 1. `auto` removes the redundant, not the type

```cpp
// LAB 11-01  cpp_modern/01_auto_and_structured.cpp
auto n = v.size();            // std::size_t, not int
auto it = counts.find(key);   // iterator type
```

The reason to use `auto` is that the type on the left of a declaration is often a restatement of the right, and a wrong restatement is a silent conversion. `int n = v.size()` truncates on a 64-bit platform where `size()` is `std::size_t`; `auto n = v.size()` cannot.

The reason not to use it is when the type is the information — `auto ratio = 1 / 2;` is an `int` zero, and writing `double ratio` would have caught it. Use `auto` when the initialiser makes the type obvious; write the type when it does not.

`auto` deduces exactly as a function template would, which means it strips the reference and the top-level `const` unless you ask for them:

```cpp
auto a = v[0];          // int: a copy
auto &b = v[0];         // int &: a reference
const auto &c = v[0];   // const int &: read-only reference
auto &&d = v[0];        // int &: forwarding reference, collapses to int&
```

The fourth form is the one people avoid because it looks like an rvalue reference; for a forwarding reference the `&&` collapses to `&` when the initialiser is an lvalue, which is why `auto &&` in a range-for binds both lvalues and temporaries.

There is a case where `auto` does not give a value at all: `std::vector<bool>` is a partial specialisation that stores bits, so `v[0]` returns a proxy object rather than a `bool &`. `auto f = flags[0];` keeps the proxy, the proxy holds a pointer into the vector's bit storage, and it is as dangling as any iterator once the vector is gone. Writing `bool f = flags[0];` forces the conversion and ends the lifetime problem — an example of the rule above, since the type here is the information.

## 2. Structured bindings

```cpp
std::map<std::string, int> m;
for (const auto &[word, count] : m) { std::cout << word << ' ' << count << '\n'; }

auto [it, inserted] = m.try_emplace(key, 0);
```

The `[it, inserted]` form is the one that removes a real defect class: before C++17, `m.insert(...)` returned a pair and code often checked `inserted` wrongly, or ignored the returned iterator. The binding makes both parts visible and named.

Structured bindings also work on arrays and on any aggregate with public members:

```cpp
struct Point { double x, y, z; };
Point p{1, 2, 3};
auto [x, y, z] = p;
```

## 3. `if` and `switch` with an initialiser

```cpp
if (auto it = m.find(key); it != m.end()) { use(it->second); }
```

The declared name is scoped to the `if`, which removes the idiom of declaring an iterator before the condition and reusing it afterwards. The same form exists for `switch`, where it is genuinely useful for a lock or a lookup that the cases share.

## 4. `string_view` is a borrow, not a string

```cpp
void report(std::string_view name);      // no copy, no allocation
```

`string_view` is a pointer and a length. It does not own the characters, it cannot grow, and it must not outlive the buffer it views.

```cpp
std::string_view sv = std::string("temp");   // dangling immediately
std::string_view sv2 = s.substr(0, 10);      // s must outlive sv2
```

The second form is the subtle one: `substr` on a `std::string` returns a new `std::string`, which converts to a view whose buffer dies at the end of the expression. The two-line version is correct because the temporary lives until the next statement; the assignment to a view is a defect.

`string_view` parameters are the correct way to accept text without copying, and the biology section uses them for every FASTA line.

## 5. `std::optional` is a value that may be absent

```cpp
std::optional<std::size_t> find_gc_start(std::string_view seq);
```

The return type says "there may be no answer" in the signature. The alternative — returning `-1` or a sentinel — encodes the absence in a value that a caller can forget to check. `optional` requires a check through `has_value()` or the value itself, and `value()` throws if absent.

The cost is one byte of flag plus the size of `T`, and `optional<T&>` is not allowed — a reference has no null state to represent.

## 6. `[[nodiscard]]` and the attributes that matter

```cpp
[[nodiscard]] int parse_header(std::string_view line);
```

The compiler warns when the return value is discarded, which catches the defect where a function returns an error code and the caller ignores it. `[[nodiscard]]` on a constructor-like factory is the highest-value use, because discarding its result means the work was wasted.

`[[maybe_unused]]` on a parameter silences the warning for a parameter kept for interface compatibility, and it documents that the silence is deliberate rather than an oversight.

## 7. `constexpr` moves work to compile time

```cpp
constexpr std::size_t kMaxName = 64;
constexpr double kGasConstant = 8.314;

constexpr int popcount(std::uint32_t x) { return x == 0 ? 0 : 1 + popcount(x & (x - 1)); }
```

A `constexpr` function can be evaluated at compile time when all inputs are constant; otherwise it is an ordinary function. This is the mechanism behind `std::array<int, N>`, template non-type parameters, and the lookup tables in the bit-manipulation chapter.

`consteval` (C++20) forces compile-time evaluation; `constinit` forces static initialisation order. Neither is needed for this book's code, but a reader meeting them in a codebase should know they exist.

## 8. What modern C++ does not fix

The features above reduce defect classes; they do not remove the need to understand layout. `auto` still copies when the initialiser is an lvalue, `string_view` still dangles, `optional` still requires the check. The features make the correct code shorter, which is the whole claim — not that the correct code is automatic.

## 9. Field note: the `endl` in the loop

A program printing one line per record used `std::endl`:

```cpp
for (const auto &r : records) { std::cout << r.name << std::endl; }
```

`std::endl` is not a newline; it is a newline plus a stream flush. Flushing per line turns a buffered write into one system call per record, and on a ten-million-line output the difference is roughly a factor of thirty. The repair is `'\n'` in the loop and a single flush at the end.

The same file had `using namespace std;` at global scope in a header, which made every `std::` name visible to every translation unit that included it and, in one case, made a local `count` ambiguous with `std::count`. The two changes — `'\n'` and removing the using-directive from the header — are the most common clean-ups in transitioning a course project to a codebase.

## 10. Practise

Read `code/cpp/11_modern/01_auto_and_structured.cpp` and predict which `auto` declarations change a type relative to the explicit form. Then attempt the exercise in `labs/cpp/11_modern/`, which requires a `string_view`-based parser that never allocates and never dangles, verified by ASan.

## Continue

- [Exceptions](exceptions.md) is the other part of the modern contract.
- [STL Algorithms](algorithms.md) shows where these features appear in pipelines.
- [Capstone: Sequence Toolkit](capstone-sequence-toolkit.md) uses all of them at once.
