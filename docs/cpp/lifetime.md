# Construction and Lifetime

Construction is not assignment, and the difference is invisible until a member has no default constructor. I start with the member initialisation list because that is where the difference shows up, then destructor order, then the copy constructor and the copy assignment that I got wrong in the same way twice, then the rule of three and the rule of zero. `explicit` is the last section because implicit conversion is the failure that happens at a call site you never wrote.

This is the chapter the rest of the C++ section rests on. An object's lifetime is a sequence with a defined order: base classes, then members in declaration order, then the constructor body; and at the end, the exact reverse. Every resource-management decision later in the book is an application of that order.

## 1. Initialisation is not assignment

```cpp
// LAB 03-01  cpp_lifetime/01_ctor_order.cpp
#include <iostream>

struct Noisy {
    Noisy(const char *tag) { std::cout << "ctor " << tag << '\n'; }
    ~Noisy() { std::cout << "dtor " << tag_ << '\n'; }
    const char *tag_;
};

struct Host {
    Noisy a{"a"};
    Noisy b{"b"};
    Noisy c{"c"};
    Host() : c{"c"}, b{"b"}, a{"a"} { std::cout << "body\n"; }
};
```

The output is:

```
ctor a
ctor b
ctor c
body
```

not the order written in the initialiser list. **Members are constructed in declaration order, always.** The initialiser list order is ignored, and a compiler with `-Wall` warns (`-Wreorder`) when the two disagree. The warning exists because the mismatch means the author believes one thing about the object and the compiler does another.

The reason is mechanical: the object is a contiguous layout, so its members must be built from the lowest offset up. Declaration order is offset order.

## 2. Why the initialiser list matters

There are three cases where only the initialiser list works, and all three appear in real code:

```cpp
class Buffer {
public:
    Buffer(std::size_t n)
        : data_(n),            // construct with the right size, one allocation
          size_(n),
          owner_(nullptr) {}
private:
    std::vector<int> data_;
    std::size_t size_;
    int *owner_;
};
```

A `const` member must be initialised, because it cannot be assigned afterwards. A reference member must be initialised, for the same reason. And a member without a default constructor must be initialised, because there is nothing to assign to. Assignment in the body cannot handle any of the three, and for everything else it costs an extra construct-and-destroy cycle.

## 3. Destruction runs backwards

Members are destroyed in reverse declaration order, after the destructor body runs. Base classes are destroyed last. The reason is the mirror of the construction order: an object may not be torn down before the things it depends on.

```cpp
~Host() { std::cout << "dtor body\n"; }   // runs first
// then c, then b, then a
```

A destructor that throws is a defect: it may run during stack unwinding, and two exceptions in flight terminate the program. Destructors should be `noexcept` by default — which they are, implicitly — and anything that can fail belongs in an explicit `close()` called before destruction.

## 4. Copying, and the rule of three

The compiler generates a copy constructor, a copy assignment, a move constructor and a move assignment unless you declare them. The generated versions copy every member, which is correct exactly when every member manages itself.

```cpp
class Raw {
public:
    explicit Raw(std::size_t n) : data_(new int[n]), size_(n) {}
    ~Raw() { delete[] data_; }
private:
    int *data_;
    std::size_t size_;
};
```

Here the generated copy constructor copies the pointer, so two objects own the same block and both destructors delete it — a double free. The rule of three says: if you need a destructor, you probably need a copy constructor and a copy assignment too, because the destructor is evidence that a member is owned rawly.

```cpp
Raw(const Raw &other) : data_(new int[other.size_]), size_(other.size_) {
    std::copy(other.data_, other.data_ + size_, data_);
}
Raw &operator=(const Raw &other) {
    Raw copy(other);          // copy first: if allocation throws, *this is untouched
    std::swap(data_, copy.data_);
    std::swap(size_, copy.size_);
    return *this;             // copy's destructor releases the old block
}
```

Before accepting that form, write the naive one and break it three times:

```cpp
Raw &operator=(const Raw &other) {
    delete[] data_;                        // 1
    data_ = new int[other.size_];          // 2
    size_ = other.size_;
    std::copy(other.data_, other.data_ + size_, data_);   // 3
}
```

Line 1 is the first failure: `a = a` deletes the block that `other.data_` still points at, and line 3 then copies from freed memory. Line 2 is the second: if `new` throws `std::bad_alloc`, the object is left holding a dangling pointer, its destructor deletes it again, and one allocation is freed twice. Line 3 is the third, and it only appears once the element type is something with a throwing assignment: the object ends up with a buffer of the new size, a size field that agrees with it, and elements that were never copied.

Copy-and-swap removes all three by ordering: the new state is fully built in a temporary before anything in `*this` changes, so a throw at any point leaves the original intact, and self-assignment is harmless because the copy is made before the old block is released. The swap itself cannot fail, which is why it is the last step rather than the first.

That ordering is also why the assignment returns by reference: `a = b = c` needs the result of the inner assignment to be the object, and returning `*this` gives that without a copy.

## 5. Rule of zero, and why it is better

The modern answer is to not own anything rawly. A class whose members are `std::vector`, `std::string` and `std::unique_ptr` needs **no** destructor, **no** copy constructor, **no** assignment operator — the generated ones are correct because each member manages itself.

```cpp
class Clean {
private:
    std::vector<int> data_;
    std::string name_;
};
```

`sizeof(Clean)` is the size of a vector plus the size of a string; copying it copies the buffers deeply; destruction releases them. Nothing is declared, and nothing can be forgotten. This is the target state, and the RAII chapter explains the mechanisms that make it possible.

## 6. `explicit`, and the conversions it prevents

```cpp
class Threads {
public:
    explicit Threads(int n) : n_(n) {}
private:
    int n_;
};

Threads t = 4;      // error with explicit, compiles without
```

An implicit single-argument constructor is a conversion operator the author did not intend. It makes `f(Threads)` callable with an `int`, which is rarely wanted and occasionally catastrophic — a function taking a `std::string` called with a `const char *` is the same pattern.

## 7. Field note: the members that were assigned in the body

The original `person.cpp` in the archive constructed its object like this:

```cpp
Person::person::person() {
    person::name = "";
    person::age = 0;
    person::sex = 'M';
}
```

Three assignments in the body, where an initialiser list belongs. Two costs follow immediately: `std::string name` is default-constructed and then assigned, which is an allocation followed by a deallocation for every object; and if a member were `const` or a reference, the code would not compile at all.

The corrected form is one line:

```cpp
Person::person::person() : name_(""), age_(0), sex_('M') {}
```

The same file has a second defect worth recording: `person.h` opens with `#ifndef PERSON_H` and never defines the macro, so the guard does nothing and only `#pragma once` prevents double inclusion. A header guard without its `#define` is a guard that has been copied and half-edited, which is exactly the kind of defect that survives for years because every compiler in use supports `#pragma once`.

## 8. Practise

Build `code/cpp/03_lifetime/01_ctor_order.cpp`, predict the output before running it, then move `c` to the front of the declaration list and predict again. The program asserts that members initialise in declaration order regardless of the initialiser list, that `constructed` is zero after a `const`-reference call, and that copies are counted rather than assumed. After that, `labs/cpp/03_lifetime/` asks you to add a copy constructor and a destructor to a class that owns a `new[]` buffer, and to check under AddressSanitizer that no block is freed twice.

## Continue

- [Inheritance](inheritance.md) inserts a base subobject at the front of this sequence.
- [RAII and Smart Pointers](raii.md) removes the need for the rule of three.
- [Exceptions](exceptions.md) explains why the copy-and-swap form above is exception-safe and the naive form is not.
