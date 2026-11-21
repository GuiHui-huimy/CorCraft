# C++ Labs

Same layout as the C labs — one directory, one `check.sh`:

```
labs/cpp/<stage_number>_<stage_name>/
└── check.sh      builds one target with the C++20 flags, runs it, exits 0
```

The task lives in `code/cpp/<stage>/README.md` and in the Practise section of
the chapter. The C++ pages all end with a Practise section for this reason: a
chapter about move semantics that the reader never compiles is a chapter about
vocabulary, not about behaviour.

`check.sh` builds with the project flags, so AddressSanitizer and
UndefinedBehaviorSanitizer are active and `-Wall -Wextra -Wpedantic
-Wconversion -Wshadow` are in the command line. The script reports any warning
it sees rather than failing on it: a strict-warning build that aborts the run
teaches nothing about the program that produced the warning. The chapter's
acceptance rule is still zero warnings, and the C++ stage READMEs repeat it.

Two labs are worth reading before running:

- `05_polymorphism/02_slicing.cpp` passes a derived object by value to a
  function that takes the base class by value, and the assertion on what the
  call prints is what fails if the copy is not noticed.
- `12_exceptions/01_stack_unwind.cpp` prints the destructor order during
  unwinding; the order, not the printed message, is the content.

`09_containers/01_invalidation.cpp` also takes an optional `--unsafe`: without
it, every iterator stays valid; with it, a pointer outlives a `push_back`
reallocation. The check runs the guarded path.

Some labs are judged on assertions inside the program rather than on exact
output, because a formatted double differs across platforms. Those labs print
their own PASS lines and exit non-zero through `CRAFT_ASSERT`.
