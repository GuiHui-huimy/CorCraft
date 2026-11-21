# STL Containers

A `vector` is three pointers, and once you accept that, every rule about invalidation becomes arithmetic. I start there, then capacity growth, then the invalidation table, then how I choose a container from the access pattern rather than from the name, then `string` as the character container, then `map` against a sorted `vector` with a measurement. The page is deliberately not a catalogue; four containers cover most of what I write.

The containers are the reason most people write C++. They are also the reason most people have a surprising number of copies, an invalidated iterator, and a `std::map` where a `std::vector` would have been faster. This page is about the three things that actually matter: layout, growth, and invalidation.

## 1. `std::vector` is three pointers

```cpp
// LAB 09-01  cpp_containers/01_invalidation.cpp
std::vector<int> v;
std::cout << sizeof(v) << '\n';     // 24 on x86-64: begin, end, capacity end
```

Three pointers, no more. `size()` is `end - begin`, `capacity()` is `cap_end - begin`. The storage lives on the heap; the object itself is 24 bytes and can be moved between scopes at that cost.

`push_back` is amortised constant time: when capacity is exhausted the vector allocates a larger block — typically 2× with libstdc++ and 1.5× with MSVC — moves or copies the elements, and releases the old block. The growth factor matters: 2× guarantees amortised O(1) but wastes up to 50% memory; 1.5× wastes less and can reuse freed blocks better. Both are legal, which is why a program's memory profile can depend on the standard library it was built against.

The reuse argument is worth following once, because it explains why the two libraries disagree. Sizes in a 2× sequence go 1, 2, 4, 8, … and each new block is larger than the sum of every block released before it (1 + 2 + 4 = 7, and the next request is 8). No freed block, and no coalescing of freed blocks, can satisfy that request, so every growth asks the system for new memory. In a 1.5× sequence the sum of earlier blocks overtakes the next request (1 + 1.5 = 2.5 > 2.25), so an allocator that coalesces adjacent free blocks can often satisfy the growth from space it already owns. libstdc++ accepts the fragmentation for a simpler size sequence; MSVC accepts the extra arithmetic for better reuse.

```cpp
v.reserve(1000);      // one allocation instead of log2(1000) reallocations
```

`reserve` is the single most effective optimisation in this chapter when the final size is known, and it is also the reason `push_back` in a loop with a known count is a code smell.

## 2. Invalidation, the rule that bites

| Operation | What it invalidates |
| --- | --- |
| `push_back` that reallocates | every iterator, pointer and reference into the vector |
| `insert` / `erase` in the middle | every iterator at or after the position |
| `reserve` | every iterator, pointer and reference |
| `resize` smaller | iterators past the new end |

```cpp
for (auto it = v.begin(); it != v.end(); ++it) {
    if (*it == 3) { v.erase(it); }        // UB: it is invalid after erase
}

v.erase(std::remove(v.begin(), v.end(), 3), v.end());   // the correct idiom
```

The erase-remove idiom is covered again in the algorithms chapter, because it is a library idiom rather than a container feature. The reason it works: `remove` moves the kept elements to the front and returns the new logical end, and `erase` then destroys the tail in one call.

The classic defect — `v.push_back(x)` inside a range-for over `v` — is undefined behaviour that often *appears* to work, because the reallocation moved the block but the stale iterator still points at freed memory that has not yet been reused. AddressSanitizer turns it into a heap-use-after-free, which is why the labs run under it.

## 3. Choosing a container

| Container | Layout | Lookup | Insert at end | Iterator stability |
| --- | --- | --- | --- | --- |
| `vector` | contiguous | O(n) | amortised O(1) | invalidated by growth |
| `deque` | blocks of contiguous | O(n) | O(1) both ends | stable except erased element |
| `list` | doubly linked | O(n) | O(1) anywhere | stable except erased element |
| `map` | red-black tree | O(log n) | O(log n) | stable |
| `unordered_map` | hash buckets | O(1) average | O(1) average | invalidated by rehash |

The default is `vector`. It wins on cache locality for almost every size that fits in memory, and the O(n) lookup is irrelevant when n is small or when the data is scanned anyway. `map` is for ordered iteration and stable iterators, not for "fast lookup" — a sorted `vector` with `std::lower_bound` is usually faster for read-heavy work and is what the sequence-alignment chapter uses.

`unordered_map` is a hash table with separate chaining in libstdc++ and a node-based design in MSVC; iteration order is unspecified and changes on rehash, so any code that depends on order is a defect. `list` is almost never the right answer: the pointer chasing costs more than the O(1) insert saves, and the memory overhead is two pointers per element.

## 4. `std::string` is a vector of characters

`std::string` has the same three-pointer layout plus a small-string optimisation in some implementations. It is contiguous, it grows the same way, and its iterators invalidate the same way. `c_str()` returns a pointer into the buffer that is invalidated by any operation that reallocates.

```cpp
const char *p = s.c_str();
s += "more";        // may reallocate; p is now dangling
```

This is the most common string defect in code that mixes C and C++ APIs: the `const char *` is handed to a C function, the string is modified, and the pointer is used again. The fix is to keep the C call adjacent to the `c_str()` call, or to copy into a local `std::string` first.

## 5. Field note: the map that was a vector

The archived word-count program used `std::map<std::string, int>` and incremented per word:

```cpp
std::map<std::string, int> counts;
while (in >> word) { ++counts[word]; }
```

Correct, and slow for a reason worth understanding: each `operator[]` walks a red-black tree of string comparisons, and each node is a separate allocation, so a 10 MB text touches thousands of cache lines in a random order. Replacing it with a `vector<std::string>` collected first and sorted afterwards changed the runtime by more than an order of magnitude on the test corpus, and produced the same output.

The generalisation: a hash map is faster than a tree map, a sorted vector is often faster than both, and the container should be chosen after measuring rather than by reputation.

## 6. Practise

Run `code/cpp/09_containers/01_invalidation.cpp` under AddressSanitizer and read the report — the stale-iterator case is deliberately included. Then attempt the exercise in `labs/cpp/09_containers/`, which asks for a word counter that is correct under reallocation and reports its own peak memory.

## Continue

- [STL Algorithms](algorithms.md) is the erase-remove idiom in context.
- [Construction and Lifetime](lifetime.md) explains why a vector copies elements on growth.
- [Sorting and Cost](../c/sorting-and-cost.md) is the C-side measurement of the same asymptotics.
