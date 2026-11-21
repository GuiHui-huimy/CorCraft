# RAII and Smart Pointers

RAII is one sentence — a destructor runs when the object's lifetime ends — and everything else in this chapter follows from it. I teach it with a class of my own before any standard type, because the mechanism matters more than the header. Then `unique_ptr`, then `shared_ptr` and the control block, then move semantics, then the section that says a smart pointer is not a substitute for thinking about ownership.

RAII is the answer to chapter 7's obligation. Instead of one owner who remembers to call `free`, the object owns a resource and releases it in its destructor. Because destructors run when a scope exits — including during an exception — the release happens on every path, not only the paths the author remembered.

## 1. The mechanism in one class

```cpp
// LAB 08-01  cpp_raii/01_scope_guard.cpp
class Buffer {
public:
    explicit Buffer(std::size_t n) : data_(new int[n]), size_(n) {}
    ~Buffer() { delete[] data_; }
    Buffer(const Buffer &) = delete;             // no copying: one owner
    Buffer &operator=(const Buffer &) = delete;
private:
    int *data_;
    std::size_t size_;
};
```

Three lines do the work: the constructor acquires, the destructor releases, and the deleted copy operations make the ownership unique. The object is now correct under every exit path, including the ones added later.

`= delete` is the C++11 form. Pre-C++11 code declared the copy operations private and left them undefined, which produced a link error instead of a compile error at the call site.

## 2. `unique_ptr`: the default

```cpp
std::unique_ptr<Buffer> b = std::make_unique<Buffer>(128);
```

`unique_ptr` is a pointer with the destructor built in, and it is non-copyable by design. Passing it to a function that must not take ownership uses a reference:

```cpp
void consume(const Buffer &b);       // borrows, does not own
void take(std::unique_ptr<Buffer> b); // takes ownership: the caller must std::move
```

`std::move` at the call site is not a cast; it is a statement that the caller is giving up the object. After the move, the source pointer is null, and using it is defined — it is a null pointer, not a dangling one.

The size cost is zero: `sizeof(std::unique_ptr<T>)` equals `sizeof(T *)` for the default deleter. The deleter is a template parameter, so a custom deleter that is empty costs nothing either.

## 3. `shared_ptr`: when ownership is genuinely shared

```cpp
std::shared_ptr<Model> m = std::make_shared<Model>();
auto copy = m;      // two owners, one control block
```

`shared_ptr` keeps a reference count in a separate control block; the object is destroyed when the last owner releases. The costs are real: an atomic increment and decrement per copy, a second allocation for the control block (avoided by `make_shared`), and an indirection to reach the object.

`shared_ptr` is the right choice when two unrelated parts of a program genuinely need to keep the object alive, and the wrong choice as a default. A design in which everything is a `shared_ptr` hides the ownership graph rather than expressing it, and cycles in the graph leak because the counts never reach zero.

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;      // weak breaks the cycle
};
```

`weak_ptr` observes without owning: it does not change the count, and `lock()` returns a `shared_ptr` or null if the object is gone.

## 4. Move semantics, at the level RAII needs

A class that owns a resource can often be moved instead of copied, which turns a deep copy into a pointer transfer:

```cpp
class Buffer {
public:
    Buffer(Buffer &&other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;      // the moved-from object must still be destructible
        other.size_ = 0;
    }
    Buffer &operator=(Buffer &&other) noexcept { ... }
};
```

The `noexcept` is load-bearing: a `std::vector` reallocating its storage will move elements only if the move constructor is `noexcept`; otherwise it copies, because a throwing move during reallocation cannot be undone.

The moved-from object must be left in a valid state — destructible, assignable, and with no ownership of the transferred resource. Setting the pointer to null accomplishes exactly that, since `delete[] nullptr` is defined.

## 5. Smart pointers are not a substitute for a destructor

A class holding a `FILE *` still needs a destructor, because `unique_ptr`'s default deleter calls `delete`, not `fclose`:

```cpp
struct FileCloser { void operator()(std::FILE *fp) const { if (fp) std::fclose(fp); } };
std::unique_ptr<std::FILE, FileCloser> fp(std::fopen(path, "rb"), FileCloser{});
```

The custom deleter is a function object, empty in this case, so the `unique_ptr` remains pointer-sized. This is the pattern for every C resource: sockets, handles, `HDF5` identifiers, `MPI` communicators.

## 6. Field note: the three defects in one stack class

The archived `Stack.cpp` contained three independent errors in about thirty lines:

```cpp
this.data = new T[n];        // 1. missing ->, so this.data is a syntax error
delete this.data;            // 2. should be delete[], and it deletes a member of *this
if (!isFull) { ... }         // 3. calls the function pointer's truth value, always true
```

The first is a compile error and therefore harmless. The second is undefined behaviour: `delete` on an array allocated with `new[]` fails to run the destructors of the elements and may corrupt the allocator. The third is worse, because it compiles: `isFull` without parentheses converts the member function to a pointer, which is never null, so `!isFull` is always `false` and the guard never runs — the stack silently overruns its capacity.

The repair for all three is a single change of representation: use `std::vector<T>` as the member, and the allocation, the deallocation, the capacity check and the copy semantics all become correct at once.

```cpp
template <typename T>
class Stack {
public:
    explicit Stack(std::size_t cap) : data_(cap) {}
    void push(const T &v) { data_.push_back(v); }   // vector grows; no fixed capacity
private:
    std::vector<T> data_;
};
```

The lesson is the chapter's thesis: the best fix for a resource defect is to stop holding the resource manually.

## 7. Practise

Run `code/cpp/08_raii/01_scope_guard.cpp`, then delete the explicit destructor and see what LeakSanitizer reports. Then attempt the exercise in `labs/cpp/08_raii/`, which requires a `unique_ptr`-owning class that survives a throw in the middle of its constructor — the case a destructor alone cannot handle, because the object was never fully built.

## Continue

- [STL Containers](containers.md) are RAII objects with more machinery.
- [Exceptions](exceptions.md) explains why the constructor that throws is the hardest RAII case.
- [Error Handling](../c/error-handling.md) is the C-side contract this chapter replaces.
