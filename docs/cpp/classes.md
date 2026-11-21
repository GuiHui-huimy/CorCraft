# Classes and Encapsulation

A class is a struct with a hidden first parameter, and I wish someone had told me that before I learned the access keywords. The page builds one type in the order the compiler sees it: the implicit `this`, then access control as a compile-time boundary, then the header/source split, then `static` members, then `const` member functions, and finally the rule I use to decide where an interface ends — one class, one header, one obvious responsibility.

A class is a struct with access control and member functions. The second part is the one that changes the object model: a member function is an ordinary function with an extra hidden parameter, and every use of `this` is a use of that parameter.

## 1. The hidden parameter

```cpp
struct Point {
    double x, y;
    double norm() const { return std::sqrt(x * x + y * y); }
};

Point p{3, 4};
p.norm();
```

The compiler generates something equivalent to:

```cpp
double Point_norm(const Point *this) { return std::sqrt(this->x * this->x + this->y * this->y); }
Point_norm(&p);
```

That rewrite explains three facts at once. Member functions do not increase the size of the object — there is one copy of the code, not one per instance. `this` is a pointer, so `*this` is the object and `this->x` is a member access. And a `const` member function is one whose hidden parameter is `const Point *`, which is why it cannot modify members.

Before I give the number, guess three sizes on x86-64: `Point`, a class holding one `char` and no functions, and a class holding one `char` and one virtual function. The answers are 16, 1 and 16, and only the first is obvious. When I first ran this, I had the third wrong: I expected 9, because a `char` plus a pointer is nine bytes, and forgot that the object's alignment has to match its widest member.

`Point` is 16 because it is two `double`, and the member function contributed nothing — there is one `norm` in the binary, not one per object. The single-`char` class is 1 because the standard requires every object of a class type to have a distinct address, so the compiler cannot shrink it to zero even though it holds one byte. Add a virtual function and the same class becomes 16: a pointer to the vtable is stored at offset 0, the `char` follows at offset 8, and the remaining seven bytes are padding inserted so the object's alignment matches the pointer's. That is the whole of the object model in three numbers: members cost their size, functions cost nothing per object, and virtual dispatch costs one pointer per object plus padding.

## 2. Access control is compile-time, not run-time

```cpp
class Account {
public:
    explicit Account(double initial) : balance_(initial) {}
    double balance() const { return balance_; }
    void deposit(double amount);
private:
    double balance_;
};
```

`private` prevents the compiler from emitting code that touches `balance_` from outside the class. It does not prevent anyone from reading the memory: `reinterpret_cast` and pointer arithmetic can reach it, and a debugger will show it. Encapsulation is a contract enforced by the type system, not a security mechanism.

The convention that makes it useful is the trailing underscore on members, which lets a parameter and a member share a name without shadowing confusion:

```cpp
void Account::deposit(double amount) { balance_ += amount; }
```

`explicit` on the single-argument constructor is the second half of a useful class. Without it, `Account a = 100.0;` compiles as an implicit conversion; with it, the same line is an error and the author must write `Account a(100.0);`.

## 3. Declaration in the header, definition in the source

```cpp
// account.h
class Account {
public:
    void deposit(double amount);
private:
    double balance_;
};

// account.cpp
#include "account.h"
void Account::deposit(double amount) { balance_ += amount; }
```

Splitting the two is what keeps compile times sane: change the body of `deposit` and only `account.cpp` recompiles; change the class layout and every file that includes the header recompiles. That is the reason headers should contain declarations and, where possible, no definitions.

Defining a member function inside the class body makes it implicitly `inline`, which is fine for one-line accessors and a poor choice for anything with a loop. The compiler may refuse to inline it and the definition then appears in every translation unit that includes the header.

## 4. `static` members have no `this`

```cpp
class Counter {
public:
    static int total() { return total_; }
    static void reset() { total_ = 0; }
private:
    static int total_;
};

int Counter::total_ = 0;   // definition, exactly once, in a .cpp file
```

A `static` member function has no hidden parameter, so it cannot touch non-static members and it can be called without an object: `Counter::total()`. A `static` data member is one object shared by every instance — the file-scope variable from the C section, with a name scoped to the class.

C++17 allows the definition to be written inline in the header:

```cpp
inline int Counter::total_ = 0;
```

which removes the "one definition in one `.cpp`" rule and the link error that follows from forgetting it. The book uses the C++17 form in examples and mentions the older form because it is what most existing code does.

## 5. `const` member functions, and the mutable exception

A `const` member function promises not to modify the object. It can be called on a `const` object, which is why accessors are `const` and mutators are not.

```cpp
class Cache {
public:
    int lookup(int key) const {
        ++hits_;                 // error: modifying a member in a const function
        return value_;
    }
    mutable int hits_;           // deliberately exempt from the promise
};
```

`mutable` exists for exactly this case: a member that is logically not part of the object's observable value — a cache, a counter, a mutex — and may change inside a `const` function. Overusing it defeats the promise, which is the reason the keyword is rare in good code.

The const-correctness habit is worth forming early: mark every member function `const` unless it modifies the object, and mark every parameter `const &` unless it is modified or copied. Retrofitting constness onto an existing class is a day of work; writing it from the start costs nothing.

## 6. Interfaces: one class per header

```cpp
// sequence.h
#pragma once
#include <cstddef>
#include <string>

class Sequence {
public:
    explicit Sequence(std::string bases);
    std::size_t size() const;
    double gc_content() const;
private:
    std::string bases_;
};
```

A header that declares one class, its member functions, and nothing else is the shape that keeps a codebase navigable. The alternatives — several unrelated classes in one header, or a header that includes five others because the class holds them by value — both increase the chance that a change ripples through the build.

Holding members by value in the header requires their complete types, which is why `#include <string>` appears. A pointer or reference member needs only a forward declaration, which is the one legitimate use of `class Foo;` at the top of a header.

## 7. Field note: the constructor that assigned instead of initialising

An early version of a class wrote this:

```cpp
class Config {
public:
    Config(const std::string &path, int threads) {
        path_ = path;          // assignment in the body
        threads_ = threads;
    }
private:
    std::string path_;
    int threads_;
};
```

It compiles, works, and costs more than it should: every member is default-constructed first, then assigned. For a `std::string` that is an extra allocation and an extra deallocation per object. And for a member without a default constructor it does not compile at all.

The corrected form is the initialiser list, which the next chapter takes apart:

```cpp
Config(const std::string &path, int threads) : path_(path), threads_(threads) {}
```

The lesson is not stylistic. Assignment-in-body is a different sequence of operations from initialisation, and the difference is visible in the constructor's cost and in whether some members can be initialised at all.

## 8. Practise

Take `code/cpp/02_classes/01_interface_split.cpp`, which contains a working class with everything public, and split it into a header and a source file. The exercise in `labs/cpp/02_classes/` is to include that header from two translation units, make every non-mutating member `const`, and keep the data private behind accessors — the compiler reports a missing `const` only when a `const` object calls the member, which is why the second translation unit exists.

## Continue

- [Construction and Lifetime](lifetime.md) explains why the initialiser list is not a style choice.
- [Inheritance](inheritance.md) adds a base subobject to the layout this chapter established.
- [RAII and Smart Pointers](raii.md) replaces the manual release with a destructor.
