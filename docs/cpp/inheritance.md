# Inheritance

Every inheritance rule I know falls out of one fact: a derived object contains a complete base object at its start, followed by its own members. So I measure the base subobject first, then the constructor and destructor chain, then `protected` as a narrower door rather than a default, then the non-virtual destructor that leaks silently, then name hiding, then slicing, and multiple inheritance last because the diamond is the only place the layout becomes visible in a way you cannot ignore.

Inheritance is a layout decision before it is an abstraction. A derived object contains a complete base object at its start, followed by its own members, and every rule about construction, destruction, slicing and casting follows from that layout.

## 1. The base subobject

```cpp
// LAB 04-01  cpp_inheritance/01_layout_inherit.cpp
struct Base { int a; };
struct Derived : Base { int b; };

Derived d;
std::cout << sizeof(Base) << ' ' << sizeof(Derived) << '\n';   // 4 8
std::cout << &d << ' ' << static_cast<Base *>(&d) << '\n';     // same address
```

`Derived` is 8 bytes: four for `Base::a` at offset 0, four for `Derived::b` at offset 4. The address of the derived object and the address of its base subobject are identical here, which is what makes a `static_cast` from `Derived *` to `Base *` free. It is not free in every case — a base with virtual functions adds a vptr, and multiple inheritance places the second base at a nonzero offset — but the single, non-virtual case is the common one.

Adding a virtual function to `Base` changes `sizeof(Derived)` to 16 on x86-64: 8 for the vptr, 4 for `a`, 4 for `b` (with padding). The vptr is the next chapter's subject; here the point is that the layout is the thing to measure, not the thing to assume.

## 2. Construction and destruction chains

Construction runs base first, then members, then the derived constructor body:

```cpp
struct Base { Base() { std::cout << "Base\n"; } };
struct Derived : Base {
    Derived() { std::cout << "Derived\n"; }   // Base already built
};
```

Destruction runs the reverse: derived body, derived members, base. The asymmetry is deliberate, because a derived destructor may use members of the base that must still be alive; it is also the reason a base destructor must not assume the derived part is still intact.

The base constructor is selected in the initialiser list, and that is the only place it can be selected:

```cpp
Derived(int n) : Base(n), b_(n) {}
```

Omitting it calls the base's default constructor, which fails if there is none.

## 3. `protected` is a narrow door

| Access | Same class | Derived class | Anyone else |
| --- | --- | --- | --- |
| `public` | yes | yes | yes |
| `protected` | yes | yes | no |
| `private` | yes | no | no |

`protected` exists so a derived class can reach the base's internals without exposing them publicly. In practice it is the most fragile of the three, because it makes the base's representation part of the base's contract with every derived class, and changing it breaks code the base author cannot see.

The convention that works is `private` members plus `protected` accessor functions, which keeps the representation private while giving derived classes a stable interface. Public data members are acceptable in a `struct` used as a bundle and not in a class with behaviour.

## 4. Non-virtual destructors and the leak they cause

```cpp
struct Base { ~Base() {} };                      // not virtual
struct Derived : Base { std::vector<int> v; };

Base *p = new Derived;
delete p;                                        // undefined: ~Derived never runs
```

`delete p` is a two-part operation, and both parts read the static type. First it calls the destructor that `Base *` selects, which is `~Base`; `~Derived` never runs, so the vector's destructor never runs and its buffer is never released. Second it calls `operator delete`, and the size handed to it is computed from `sizeof(Base)` unless a sized deallocation for the dynamic type is available — and with a non-virtual destructor there is no way to reach `~Derived`, hence no way to learn the real size. The block was allocated as `sizeof(Derived)` and may be released as `sizeof(Base)`, which is undefined behaviour before it is even a leak.

The compiler may warn (`-Wdelete-non-virtual-dtor` with `-Wall` in GCC and Clang), and the warning is one of the few that is almost always right.

The rule: a class intended to be deleted through a base pointer needs a virtual destructor. A class not intended for that does not, and adding one costs a vptr to every object.

```cpp
struct Base { virtual ~Base() = default; };
```

## 5. Name hiding is not overriding

```cpp
struct Base { void f(int); };
struct Derived : Base { void f(double); };   // hides Base::f(int), does not overload it

Derived d;
d.f(1);        // calls Derived::f(double); Base::f(int) is invisible
```

Overload resolution happens within one scope. A derived-class function with the same name as a base-class function hides every base overload, regardless of parameters. The repair is a `using` declaration:

```cpp
struct Derived : Base {
    using Base::f;      // bring the base overloads back into this scope
    void f(double);
};
```

This is one of the few cases where a `using` declaration inside a class is necessary rather than stylistic, and it appears in real code whenever a derived class adds an overload.

## 6. Slicing, in one line

```cpp
Derived d;
Base b = d;         // slices: only the Base part is copied
```

Assignment to a base object copies the base subobject and discards the rest. If `Derived` held a `std::vector`, the slice has no vector; if it held a `std::string`, the slice has a copy of the base's string and nothing else. No warning, no error, and the object looks valid.

Slicing is the reason polymorphic interfaces are passed by reference or pointer, never by value. The chapter on polymorphism returns to this with a virtual function, where the consequence is worse: the sliced object calls the base's implementation, so the polymorphism silently disappears.

## 7. Multiple inheritance, and the diamond

Two bases place the second at a nonzero offset, which is why a cast from `Derived *` to the second `Base *` adjusts the pointer. If both bases derive from a common ancestor, the object contains two copies of that ancestor unless the inheritance is `virtual`:

```cpp
struct A { int x; };
struct B : virtual A {};
struct C : virtual A {};
struct D : B, C {};        // one shared A, reached through a pointer
```

Virtual inheritance solves the duplicate-ancestor problem and introduces a cost: the path from `D` to `A` is indirect, so the cast is a load rather than an addition. The book uses multiple inheritance exactly once, in the structure model, and the page states why.

## 8. Field note: the base class that was copied by value

A function took its parameter by value:

```cpp
void report(Shape shape);        // the original signature
```

`Shape` was an abstract base with a virtual `area()`. Every call sliced the argument, so `shape.area()` always called the base's implementation — which, being pure virtual, meant the call was resolved to a stub that returned zero. The function printed zero for every shape and nobody noticed, because zero is a plausible area for an empty polygon.

The repair was one character: `const Shape &`. The lesson is that a polymorphic type in a by-value parameter is not a subtle bug; it is a category error, and the compiler cannot catch it because slicing a base is legal.

## 9. Practise

Run `code/cpp/04_inheritance/01_layout_inherit.cpp` and add a virtual function to `Base`; observe what happens to `sizeof(Derived)` and to the addresses printed. Then attempt the exercise in `labs/cpp/04_inheritance/`, which requires a base with a virtual destructor and a derived class whose members are released exactly once, verified under AddressSanitizer.

## Continue

- [Polymorphism](polymorphism.md) explains the vptr that changed the layout above.
- [Construction and Lifetime](lifetime.md) is the sequence this chapter extended.
- [Exceptions](exceptions.md) covers what happens when a constructor throws midway through a chain.
