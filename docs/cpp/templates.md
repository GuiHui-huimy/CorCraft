# Templates

A template is not a type; it is a recipe the compiler follows once it knows the argument types, and until then the body is barely checked. That is why the errors arrive late and why this chapter spends its space on diagnosis. I begin with deduction, then a class template with a non-type parameter, then lazy instantiation and the errors it hides, then two-phase lookup, then concepts as the constraint that turns a page of errors into a sentence. The Field Note is a template that compiled and returned a reference to a temporary.

A template is a recipe the compiler follows when it has the argument types. Until then the body is not checked for much of anything, which is why template errors arrive late, arrive with long messages, and are the reason this chapter spends most of its space on diagnosis rather than syntax.

## 1. Function templates and deduction

```cpp
// LAB 07-01  cpp_templates/01_template_diag.cpp
template <typename T>
T larger(T a, T b) { return a > b ? a : b; }

larger(3, 4);          // T = int
larger(3.0, 4.0);      // T = double
larger(3, 4.0);        // error: conflicting deduction, T cannot be both
```

Deduction works on the declared parameter types, not on the argument values. Passing an `int` and a `double` for two `T` parameters asks the compiler to deduce `T` twice with different answers, which is an error rather than a conversion. The fixes are a cast at the call site or two template parameters with a `std::common_type_t`.

Explicit arguments bypass deduction:

```cpp
larger<double>(3, 4.0);   // T = double, the int converts
```

## 2. Class templates and non-type parameters

```cpp
template <typename T, std::size_t N>
class FixedBuffer {
public:
    explicit FixedBuffer(const T (&src)[N]) { std::copy(src, src + N, data_); }
    std::size_t size() const { return N; }
private:
    T data_[N];
};
```

`N` is a non-type parameter: it must be a compile-time constant, and it becomes part of the class's identity. `FixedBuffer<int, 4>` and `FixedBuffer<int, 8>` are unrelated types with unrelated layouts. This is the mechanism behind `std::array`, and it is why `std::array<int, 4>` has no dynamic allocation and no runtime size field.

## 3. Instantiation is lazy, and that is the source of the late error

A class template's member functions are instantiated only when used. A function that is never called is never compiled, so a defect inside it is invisible until someone calls it — and the error then points at the call site rather than at the defect.

```cpp
template <typename T>
T minimum(T *arr, std::size_t n) {
    T min = arr[0];
    for (std::size_t i = 0; i < n; ++i) {
        if (arr[0] < min) { min = arr[0]; }   // never true: compares arr[0] with itself
    }
    return min;
}
```

This function compiles for every `T` that supports `<`, returns `arr[0]` for every input, and produces no warning. The body looks plausible, the loop variable `i` is unused in the condition, and the flaw is a leftover from a version that compared `arr[i]`.

The reason it survived is exactly the laziness: nothing in the template's declaration suggests a problem, and the call site gets a correct-looking value, so no error is ever raised. A test with a three-element array where the minimum is not first would have caught it immediately.

## 4. Two-phase lookup

Names inside a template are looked up in two phases, and the split explains a class of errors:

1. **At definition**: names that do not depend on a template parameter must be visible, along with everything from the enclosing namespaces.
2. **At instantiation**: names that depend on a template parameter are resolved, including argument-dependent lookup (ADL).

```cpp
template <typename T>
void scale(T &v, double f) { v.scale(f); }   // v.scale resolved at instantiation
```

A dependent call is resolved at instantiation, which is why a free function found only by ADL works and a function declared after the template definition may not. The practical rule: declare everything the template needs before the template, or move the definition into the same header as the declaration.

The single most common symptom is "the compiler says my function does not exist, but it clearly does". The declaration order is usually the answer.

## 5. Constraints, and what concepts replace

Before C++20, constraining a template meant `static_assert` inside the body or SFINAE in the signature. Both produce errors that are hard to read:

```cpp
template <typename T>
T sum(const std::vector<T> &v) {
    static_assert(std::is_arithmetic_v<T>, "sum requires an arithmetic type");
    ...
}
```

C++20 concepts move the constraint into the signature and improve the diagnostic:

```cpp
template <std::arithmetic T>
T sum(const std::vector<T> &v);
```

The book uses concepts in the capstone and mentions the older forms because existing code is full of them. The diagnostic quality difference is large enough to be worth the standard bump.

## 6. Field note: the sort that sorted backwards

The archived `funTemplate.h` contained a class template whose `sort()` member compared and swapped like this:

```cpp
if (a[i] < a[j]) {
    T temp = a[i];
    a[i] = a[j];
    a[j] = temp;
}
```

With `i < j` and the condition `a[i] < a[j]`, the larger element moves to the earlier position, so the result is descending order. The function is named `sort`, so a caller expecting ascending order gets an array that is sorted and wrong.

There was no test, because a template has no obvious place for one, and the only caller was a commented-out `main`. Two lessons: name a function for what it guarantees, and give a template at least one instantiation that runs in the test suite — otherwise it is code that has never been compiled.

The same file's `Stack` template had the sibling defect: an array allocated with `new[]` and released with `delete` rather than `delete[]`, which is undefined behaviour and reported by ASan as an allocation/deallocation mismatch.

## 7. Practise

Build `code/cpp/07_templates/01_template_diag.cpp`, which contains the sort defect above, and make its assertions pass: ascending order for `int`, `double` and `std::string`, a minimum of `1` from a five-element array, `w` descending as `5 … 1`, and `larger<double>(3, 4.5)` resolving to `4.5` only when the argument is explicit.

## Continue

- [RAII and Smart Pointers](raii.md) is what a template like `FixedBuffer` needs instead of a raw array member.
- [STL Containers](containers.md) are templates instantiated far more often than you might expect.
- [Construction and Lifetime](lifetime.md) explains why `FixedBuffer`'s copy constructor is implicit and correct.
