# C Labs

Each lab is one directory holding one file:

```
labs/c/<stage_number>_<stage_name>/
└── check.sh      configures code/, builds one target, runs it, exits 0
```

There is no starter and no expected-output file. The exercise is the program in
`code/c/<stage>/`, described by that stage's README and by the Lab section of the
chapter; a second scaffolded copy would only drift from it. `check.sh` exists so
that a reader whose own attempt misbehaves can prove, in one command, that the
reference program still behaves as the book says on this machine.

`check.sh` builds with the project flags, which means AddressSanitizer and
UndefinedBehaviorSanitizer are on. Four programs — `04_arrays_layout/
02_bounds_probe.c`, `07_heap/02_double_free.c`, `09_undefined_behavior/
01_signed_overflow.c` and `09_undefined_behavior/02_ub_sweep.c` — take an
optional `--unsafe` argument that performs the defect on purpose. The checks run
them without the flag, so they pass on the guarded path; adding `--unsafe`
by hand is how the reader sees the sanitizer report, and a non-zero exit there
is the expected result, not a failure.

A build that emits a warning prints the warning text and keeps going. The C
chapters aim for zero warnings under `-Wall -Wextra -Wpedantic -Wconversion
-Wshadow`; the script reports, the reader judges.

Run every lab at once from the repository root:

```bash
bash tools/run_labs.sh
```

The numeric prefix matches a stage directory under `code/c/`; the chapter page
is named in each stage's README.
