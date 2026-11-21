# STL Algorithms

The algorithms header is where the standard library stops being a container and becomes a set of loops you no longer write. I take four that matter — `sort`, `find`, `transform`, `accumulate` — and then the comparator contract, because a comparator that is not a strict weak order is undefined behaviour rather than a wrong answer. Then erase-remove, then lambdas and what they capture, then ranges as the same idea with fewer iterator pairs.

The algorithms are free functions over iterator pairs. They do not know about containers, which is why they work on raw arrays, vectors, and the sequence buffers in the biology section without any adaptation.

## 1. The four that carry most of the work

```cpp
// LAB 10-01  cpp_algorithms/01_lambda_pipeline.cpp
std::sort(v.begin(), v.end());
std::stable_sort(v.begin(), v.end(), by_score_desc);
auto it = std::find_if(v.begin(), v.end(), [](int x) { return x > 100; });
std::transform(in.begin(), in.end(), out.begin(), [](double x) { return x * 2.0; });
```

`std::sort` is introsort: quicksort with a median-of-three pivot, falling back to heapsort when recursion gets too deep, so the worst case is O(n log n) rather than O(n²). It is not stable. `stable_sort` is merge sort and allocates a temporary buffer; use it when equal elements must keep their relative order — a sort by score that must preserve the input order among equal scores.

`std::find_if` returns the iterator, and the caller must check it against `end()`. A common defect is comparing the result to the wrong container's `end()`.

`std::transform` writes through an output iterator and does not resize the destination; the destination must already have room. `std::back_inserter(out)` is the fix for a container that should grow.

## 2. The comparator must be a strict weak ordering

This is the requirement that produces the most undefined behaviour in the whole library, because violating it usually *appears* to work.

```cpp
std::sort(v.begin(), v.end(), [](int a, int b) { return a <= b; });   // UB
```

`<=` is not a strict weak ordering: it returns true for equal elements, so `comp(a, a)` is true, which no correct sort can tolerate. The result is out-of-bounds reads inside the sort — undefined behaviour, and AddressSanitizer will not catch it because the reads stay within the buffer.

The mechanism is worth seeing once. libstdc++ partitions with an unguarded loop — `while (comp(*first, pivot)) ++first;` — and it omits a bounds check because the pivot itself is supposed to stop the scan. When `comp` is `<=`, `comp(pivot, pivot)` is true, so the scan passes the pivot, then passes the end of the range, and only stops when it happens to meet a value the comparator rejects or when it reads unmapped memory. That is why the symptom is a segfault in `std::sort` with no relation to the caller's data, and why debug mode (`-D_GLIBCXX_DEBUG`) is the only reliable detector.

The correct form is `a < b`. For a descending order, `a > b`. For a tuple, `std::tie(a.x, a.y) < std::tie(b.x, b.y)`.

The second form of the same defect is an inconsistent comparator — one that orders by `x` when `x` differs and by `y` otherwise but forgets a case. The library's only defence is a debug-mode check in some implementations; in release builds the sort silently produces a wrong order or reads out of bounds.

## 3. The erase-remove idiom

```cpp
v.erase(std::remove_if(v.begin(), v.end(),
                       [](const Record &r) { return r.score < 60; }),
        v.end());
```

`remove_if` cannot erase, because it only has iterators; it moves the kept elements to the front and returns the boundary. `erase(boundary, end())` then destroys the tail. The two-call form is not a workaround, it is the intended design: the same `remove_if` works on a raw array, where there is no erase.

`std::remove` returns the new end and the container must be shrunk explicitly. Forgetting the `erase` leaves the container with the same size and moved-from elements at the tail, which compiles and runs.

## 4. Lambdas and capture

```cpp
int threshold = 60;
auto pred = [threshold](const Record &r) { return r.score < threshold; };   // by value: safe
auto counter = [&count](const Record &r) { ++count; return r.score < 60; }; // by reference: lifetime risk
```

`[=]` and `[&]` are the convenient forms and the ones that cause defects. `[&]` captures everything by reference, including things the lambda outlives — a lambda stored in a `std::function` and called after the scope has exited reads a dead local. Capturing the two or three names explicitly makes the lifetime visible in the signature.

A lambda that captures `this` (`[this]`) captures the pointer, not the object; if the object dies, the capture dangles. C++20's `[=, this]` form exists to make that explicit.

## 5. Ranges, one paragraph

C++20 ranges replace iterator pairs with a single object:

```cpp
std::ranges::sort(v);
auto evens = v | std::views::filter([](int x) { return x % 2 == 0; });
```

Views are lazy and do not own data, so a view over a temporary is a dangling view — the same lifetime question as `string_view`. The book uses ranges in the capstone and keeps the iterator form everywhere else, because the iterator form is what every existing codebase contains.

## 6. Field note: the comparator that subtracted

A comparator written as:

```cpp
bool cmp(int a, int b) { return a - b; }
```

worked for small values and produced a wrong order for large ones. `a - b` overflows `int` when the difference exceeds the range, and the sign of the overflowed result is not the sign of the true difference. `qsort` and `std::sort` both accept a comparator returning `bool`, so `a - b` converts to `true` whenever it is nonzero — meaning "less than" is reported for both `a > b` and `a < b`.

The repair is `a < b`. The general rule: a comparator returns a truth value, never a difference, and the difference form belongs to three-way comparisons where the return type is a signed integer wide enough to hold it.

## 7. Practise

Compile `code/cpp/10_algorithms/01_lambda_pipeline.cpp` with `-D_GLIBCXX_DEBUG`, which enables the library's comparator checks, and run the deliberately broken comparator to see the diagnostic. Then read `code/cpp/10_algorithms/README.md` and rewrite the program into `/tmp` from memory: sort a struct array by two keys, remove failing records with the erase–remove idiom, and assert the surviving count and the first element — the same two properties the reference program checks.

## Continue

- [STL Containers](containers.md) is where the iterators come from.
- [Sorting and Cost](../c/sorting-and-cost.md) measures the same algorithms from the C side.
- [Modern C++](modern.md) covers the features that make these pipelines shorter.
