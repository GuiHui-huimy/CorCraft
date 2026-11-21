# The Bridge from C

The first C++ page is not a tour of features; it is a list of what changed and what did not. I start with the reference, because 'a reference is a pointer in disguise' is the sentence that makes every later overload resolution misread. Then overload resolution, then namespaces, then the cost of `iostream`, then the one line I still write rarely: `new`. By the end you should be able to say which of your C habits survive the move and which quietly change meaning.

C++ is not C with classes added; it is a different language that happens to be source-compatible with most of C. The differences that matter first are not the famous ones — templates and virtual functions come later — but four small ones that change how you write every function: references, overloading, namespaces, and the fact that `main` can end without a `return`.

## 1. References are not pointers with nicer syntax

```cpp
void by_pointer(int *p) { *p = 7; }
void by_reference(int &r) { r = 7; }

int x = 1;
by_pointer(&x);   // the caller writes the address
by_reference(x);  // the caller writes the object
```

A reference is an alias for an existing object. It has no null state, it cannot be reseated, and it cannot be used for arithmetic. Those three properties are the reason C++ functions prefer it: a `const std::string &` parameter means "I will not copy this and I will not modify it", and the call site cannot accidentally pass a null.

The cost is that a reference looks exactly like the object at the call site, which hides the fact that a function may modify the argument. `by_reference(x)` above gives no visual hint that `x` changes. That is a genuine readability cost, and the convention that mitigates it is `const` on everything that does not change — a promise visible in the signature and enforced by the compiler.

```cpp
// LAB 01-01  cpp_bridge/01_ref_vs_ptr.cpp
int &pick(bool first, int &a, int &b) { return first ? a : b; }
```

Returning a reference is legal and is how `operator[]` and `std::map::at` work. Returning a reference to a local is the same defect as returning a pointer to a local, with the same consequence.

## 2. Overloading, and the ranking that decides

Two functions with the same name and different parameters are resolved by a ranking that is stricter than most readers expect:

```cpp
void show(int);
void show(double);
void show(const char *);

show(42);        // exact match: int
show(3.14f);     // float -> double promotion beats float -> int conversion
show("text");    // const char[5] decays to const char *
show('a');       // char -> int promotion, so show(int)
```

The last line is the one to remember: `'a'` is a `char`, `char` promotes to `int` exactly, and `show(int)` wins even though `show(const char *)` looks like the natural candidate. An overload set that makes this surprising is an overload set that should be renamed.

Ambiguity is a compile error, not a silent choice:

```cpp
void f(long);
void f(unsigned long);
f(0);            // ambiguous: int converts equally to both
```

The fix is an explicit cast at the call site or, better, two differently named functions.

## 3. Namespaces are a naming tool, not a scope hack

```cpp
namespace craft {
constexpr double pi = 3.14159265358979323846;
double scale(double, double);
}
```

A namespace prevents collisions between libraries, and it costs nothing at run time. The two habits that matter: never write `using namespace std;` in a header, because it imposes the entire standard library on every file that includes the header; and prefer a short namespace alias inside a `.cpp` file over a `using` directive at file scope.

`using namespace` in a header is the most common cause of an ambiguity that appears only when two headers are included together, which is a defect that shows up in a consumer's build and not in yours.

## 4. `iostream` is slower than `stdio`, and that is a choice

```cpp
std::ios::sync_with_stdio(false);
std::cin.tie(nullptr);
```

By default `std::cin` and `std::cout` are synchronised with C's `stdin` and `stdout`, which makes every read and write go through the C library's buffer as well. The two lines above disable the synchronisation; the cost is that mixing `printf` and `cout` no longer guarantees ordering, and the gain is typically a factor of two to four on input-heavy programs.

The reason to care appears in the biology section: parsing a 30,000-atom file with `cin >>` unmodified is several times slower than with `fscanf`, and the difference is not the format but the synchronisation.

For output, `'\n'` rather than `std::endl` avoids a flush per line. `std::endl` flushes, which is useful for a progress indicator and ruinous for a loop.

## 5. `new` and `delete` exist, and you should rarely use them

```cpp
int *p = new int[10];    // allocation, no initialisation
delete[] p;              // must match new[]
```

`new` throws `std::bad_alloc` on failure rather than returning null, which is the semantic difference from `malloc`. Everything else about manual `new`/`delete` is the same obligation as chapter 7 of the C section — one owner, one release — and the same defects: leaks on an early return, double delete, delete of a pointer that was not allocated by `new`.

The reason this book defers `new` to a later chapter is that the correct amount of manual `new` in modern C++ is close to zero. `std::vector`, `std::string` and `std::unique_ptr` allocate for you and release in a destructor, which is the subject of the RAII chapter. `new` remains in the text because it appears in every legacy codebase and because understanding it is a precondition for understanding what the containers do internally.

## 6. What carries over unchanged

Arrays, pointers, `sizeof`, the object model, undefined behaviour, the stack, the heap, the preprocessor and translation units. C++ does not fix any of them. It adds tools that let you avoid them, and it adds new ways to get them wrong — which is the honest summary of the next twelve chapters.

## 7. Practise

Copy `code/cpp/01_bridge/01_ref_vs_ptr.cpp` into `/tmp`, compile it, and change `scale_ref` to take a `Point *` instead of a `Point &`. Observe what happens to the call sites: how many `&` and `*` appear, whether a caller can now pass `nullptr`, and whether the function still compiles unchanged at the point where `alias = q` renames nothing. Then read `code/cpp/01_bridge/README.md` and run `bash labs/cpp/01_bridge/check.sh`: the program asserts the two properties the change breaks — a reference cannot be null, and assigning through a reference copies into the referent instead of rebinding — so the rewrite is measured against them. The rewrite is the exercise; the script only proves the baseline still holds.

## Continue

- [Classes and Encapsulation](classes.md) introduces the type that makes the rest of the language possible.
- [Construction and Lifetime](lifetime.md) starts the question that dominates the section: when does an object begin and end.
- [Compiler and Flags](../reference/compiler-flags.md) lists the C++-specific warnings this chapter assumes.
