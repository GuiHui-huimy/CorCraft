# Exceptions

An exception is a control transfer with a guarantee attached: during unwinding, destructors run. I start from that guarantee, then construction that throws, then `noexcept` as a promise about the move, then catching by reference, then the comparison with error codes that I use to decide which one a function should use. The cost section separates the zero-cost path from the table lookup, because most of the fear about exceptions is about the wrong path.

An exception is a non-local exit with a defined cleanup: the stack unwinds, every fully constructed object in the unwound frames is destroyed, and control arrives at a matching handler. That cleanup is what makes exceptions compatible with RAII and what makes them dangerous in code that has not adopted RAII.

## 1. What unwinding guarantees

```cpp
// LAB 12-01  cpp_exceptions/01_stack_unwind.cpp
void inner() {
    Buffer b(128);            // RAII
    int *raw = new int[64];   // not RAII
    throw std::runtime_error("boom");
    // b's destructor runs during unwinding; raw leaks
}
```

The destructor of `b` runs because `b` is fully constructed. The `raw` pointer leaks because a pointer is not an owner — nothing knows to release it. This is the single argument for RAII that matters: without it, every function between the throw and the catch becomes a leak site.

The guarantee is precisely stated: objects with automatic storage duration that have completed construction are destroyed, in reverse order of construction. Objects whose construction was interrupted are not destroyed — only their already-constructed members are.

## 2. The constructor that throws

```cpp
class Connection {
public:
    Connection(const std::string &host) : socket_(open_socket(host)), buffer_(new char[4096]) {}
    // if open_socket throws, socket_ is not constructed; nothing to destroy
    // if new throws, socket_ IS destroyed; buffer_ is not
private:
    Socket socket_;
    char *buffer_;
};
```

The rule is that the destructor does not run for the object whose constructor threw, but the destructors do run for its completed members. So a constructor that acquires two resources and throws after the first has already released the first correctly — as long as the first is held by an RAII member rather than a raw pointer.

This is why the labs require a constructor that throws midway: it is the case a hand-written destructor cannot handle, because the object never existed.

## 3. `noexcept`, and where it must be

```cpp
class Buffer {
public:
    Buffer(Buffer &&other) noexcept;    // required for vector to move instead of copy
};
```

A move constructor that is not `noexcept` makes `std::vector` copy on reallocation, because a throwing move cannot be rolled back. The performance difference is measurable for large element types and invisible in the code.

`vector` does not test the exception specification by hand. Reallocation calls `std::move_if_noexcept`, a function whose return type is chosen by the traits of `T`: it yields an rvalue reference when the move constructor is `noexcept`, or when `T` has no copy constructor to fall back on, and a `const T &` otherwise. So the decision is made at compile time, per element type, and the cost of a missing `noexcept` is a silent switch from moving to copying for every element in every reallocation of every vector of that type. When `std::string`'s move constructor is not marked `noexcept`, a `std::vector<std::string>` copies each element on every reallocation instead of moving it, and the cost scales with the number of reallocations and the length of each string. Nothing in the source shows it: the only difference is a missing keyword in a declaration.

A destructor is implicitly `noexcept`, and a destructor that throws during unwinding calls `std::terminate`. If a cleanup can fail, the failure belongs in an explicit `close()` that the caller invokes before destruction, with its error handled there.

`noexcept` is also an optimisation hint: a call to a `noexcept` function does not need the unwinding machinery around it, so the code is smaller.

## 4. Catch by reference, always

```cpp
try { ... }
catch (const std::runtime_error &e) { std::cerr << e.what() << '\n'; }
catch (const std::exception &e) { std::cerr << "other: " << e.what() << '\n'; }
```

Catching by value slices the exception: a derived exception type is copied into a base object and the derived part is lost. Catching by reference preserves the dynamic type and avoids a copy. `catch (...)` catches everything, including non-`std::exception` types, and should be used only to rethrow or to log at a boundary.

Order matters: handlers are tried in order, so the most derived type must come first. A `catch (const std::exception &)` before a `catch (const std::runtime_error &)` makes the second unreachable.

The complementary failure is a handler that matches nothing. `std::out_of_range` derives from `std::logic_error`, not from `std::runtime_error` — the hierarchy is `exception → logic_error → out_of_range` — so a `catch (const std::runtime_error &)` written for an out-of-range index does not catch it, and the exception reaches `std::terminate`. The repair is to catch `std::logic_error &` or `std::exception &`. `code/cpp/12_exceptions/01_stack_unwind.cpp` throws an `out_of_range` and catches it by base class for exactly this reason.

## 5. Exceptions versus error codes

| | Error codes | Exceptions |
| --- | --- | --- |
| Cost when no error | a branch | zero on the hot path |
| Cost when an error | a branch at every call site | unwinding, which is slow |
| Ignorable by accident | yes | no |
| Works across C ABI | yes | no |
| Works in a destructor | yes | dangerous |

The last two rows are the reason both exist. The C-facing boundary — a function exported to a C library, a callback registered with a C API — must not throw across it, because the C caller has no unwinder. The pattern is a `noexcept` wrapper that translates an exception into an error code:

```cpp
extern "C" int c_run(const char *path) noexcept {
    try { run(path); return 0; }
    catch (const std::exception &) { return -1; }
}
```

## 6. Performance, honestly

The zero-cost claim is about the non-throwing path: with table-based unwinding, entering a `try` block costs nothing at runtime, and the tables live in a separate section. Throwing is expensive — on the order of a microsecond for the first throw in a process, faster afterwards — which is why exceptions are for exceptional conditions and not for control flow.

A loop that throws on a missing key in a map, caught immediately, is slower than `find` plus a check by orders of magnitude. The rule: exceptions for "this cannot continue", return values for "this did not work, try another way".

## 7. Field note: the error code nobody checked

The archived `mocal.c` allocated an array of strings and checked every allocation:

```cpp
str[i] = (char *)malloc(64);
if (str[i] == NULL) { printf("malloc failed\n"); }
// ... and then continued to use str[i] anyway
```

The check printed a message and fell through. A null buffer was then passed to `strcpy`, which is undefined behaviour and, in practice, a crash. The error was *detected* and not *propagated*: the code had the information and no way to act on it, because the function returned `void`.

The C++ repair is either an exception (`throw std::bad_alloc{}`, which is what `new` does) or a return type that can express failure. Both make the caller unable to ignore the failure, which is the difference between detecting an error and handling one.

## 8. Practise

Build `code/cpp/12_exceptions/01_stack_unwind.cpp` and read the destructor order: the RAII member and the raw pointer are both released during unwinding, because `Mixed` has a destructor. Delete that destructor, rebuild, and ASan reports the block as leaked at exit — that one-line experiment is the argument for RAII. Then attempt the exercise in `labs/cpp/12_exceptions/`: a constructor that throws after acquiring a resource, asserting zero leaks and a correct error message.

## Continue

- [RAII and Smart Pointers](raii.md) is the mechanism unwinding depends on.
- [Construction and Lifetime](lifetime.md) explains which members are destroyed when a constructor throws.
- [Error Handling](../c/error-handling.md) is the C contract that the boundary wrapper translates.
