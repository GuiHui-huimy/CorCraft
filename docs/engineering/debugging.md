# Debugging

A crash is not a mystery; it is a state you have not read yet. I begin with the backtrace because it costs one command and answers most questions, then frames and prints, then breakpoints with conditions, then `printf` done properly, then bisection for wrong results, then sanitizers beside the debugger. The order is the order I actually follow at two in the morning.

The debugger is slower than `printf` and faster than guessing. This page covers the four operations that solve almost every crash in this book, and the two habits that decide whether a debugging session ends in ten minutes or in an hour.

## 1. The first crash: get a backtrace before anything else

```bash
gdb ./app
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x0000555555555163 in read_records (path=0x0) at records.c:41
41          std::fread(buf, 1, size, fp);
```

The three lines that matter are `SIGSEGV`, the function and line, and the arguments. `path=0x0` is the answer in this case: the caller passed a null path, and `fopen(NULL, ...)` returned null, and `fread` on a null `FILE *` crashed.

```bash
gdb -batch -ex run -ex bt ./app       # same, without an interactive session
```

The batch form is what goes into a script: it runs, and on a signal prints the backtrace and exits. A harness that runs binaries unattended wants this form, so that a crash produces a report rather than a timeout.

The transcripts on this page use a two-function toy — `load.c` calling `records.c` — rather than a file from `code/`. The argument does not depend on which program crashed, and a toy keeps the frames short enough to read; the two files under `code/c/16_error_handling/` are the repository's own examples of the same pattern.

## 2. `bt` is the map, `frame` and `print` are the inspection

```text
(gdb) bt
#0  read_records (path=0x0) at records.c:41
#1  load (cfg=0x7fffffffd9d0) at main.c:63
#2  main () at main.c:12

(gdb) frame 1
(gdb) info locals
(gdb) print cfg->path
```

`bt` numbered from the innermost frame. `frame N` moves the inspection context to that frame, and `info locals` and `print` then show that frame's values. The most common mistake is inspecting frame 0 when the bad value was computed in frame 1 — the crash site is rarely the defect site.

`up` and `down` move by one frame; `info args` shows the parameters; `x/8gx addr` dumps memory as eight 8-byte hexadecimal words, which is what reads a struct or a buffer correctly.

## 3. Breakpoints, and the conditional form

```text
(gdb) break records.c:41
(gdb) break read_records if size > 1000     # conditional
(gdb) info breakpoints
(gdb) delete 2
```

A conditional breakpoint is the tool that makes a loop debuggable: set it on the iteration where the invariant fails, not on the first iteration. `ignore N 1000` skips the first thousand hits, which is the same idea without a predicate.

`watch -l buf[0]` stops when the memory changes, which is how a buffer overrun is caught at the write rather than at the read that follows. `rwatch` and `awatch` are the read and read/write variants.

## 4. Printing is legitimate, and there is a right way

`printf` debugging gets a bad reputation from being used badly. Done well it is the fastest tool:

```c
fprintf(stderr, "%s:%d path=%p size=%zu\n", __FILE__, __LINE__, (void *)path, size);
```

Three rules: write to `stderr` so the output is not mixed with the program's real output; include the file and line so a later copy of the message is unambiguous; print in one statement so the line is atomic with respect to other output. A loop that prints ten thousand lines is a loop that needed a conditional print, not a smaller buffer.

The advantage over a breakpoint is that a printed log survives the run and can be diffed. The disadvantage is that it changes the timing and the code. When both are needed, `assert`-style checks plus a log are usually enough, and the debugger is for the case where the value is unknown in advance.

## 5. Bisecting a wrong result

A crash has a site; a wrong answer has a range. The technique is to narrow the range by halving:

1. Print the input and the output at the function boundary. If the input is already wrong, the defect is upstream.
2. If the boundary is clean, print at the midpoint of the function's body.
3. Repeat, halving each time. A function of 100 lines is located in 7 prints.

This is faster than stepping through the function, because each print eliminates half the remaining lines rather than one line.

## 6. Sanitizers and the debugger, together

The labs build with ASan and `-g`, so a report includes line numbers:

```bash
./app
==12345==ERROR: AddressSanitizer: heap-buffer-overflow
    #0 0x... in fill at buffer.c:18
```

ASan aborts at the first bad access, so `gdb --args ./app` followed by `run` lands in the debugger with the full stack at the moment of the illegal access — the best of both tools. `ASAN_OPTIONS=abort_on_error=1` makes that reliable.

LeakSanitizer reports at exit and does not abort, so a leak is diagnosed after the program has finished. To stop at the first leak, `ASAN_OPTIONS=detect_leaks=1:abort_on_error=1` with a breakpoint on `__lsan_do_leak_check`.

## 7. Field note: the crash that was three lines upstream

The archived `mocal.c` crashed inside `strcpy`, and the backtrace pointed at `strcpy` in libc with no useful frame.

```c
char *str[10];
for (int i = 0; i < 10; ++i) { str[i] = (char *)malloc(64); }
strcpy(str[0], "hello");
```

The crash was reported inside `strcpy`, but the defect was one line up: `if (str[i] == NULL)` printed a message and continued, so a null destination reached `strcpy`. The library function cannot know the pointer is null until it writes to address 0, and the report names the library rather than the caller.

The habit that finds this in one step: when the backtrace's innermost frame is inside libc, the defect is in the frame above it, and that frame's arguments are the ones to inspect. `frame 1` plus `print str[0]` would have shown `(char *) 0x0` immediately.

## 8. Practise

Take a lab that passes, introduce one defect of your own (a null dereference, an off-by-one write, or a leaked allocation), and diagnose it three ways: with ASan alone, with `gdb` on the sanitizer abort, and with a print bisection. Record which one found the line first.

## Continue

- [Testing and Sanitizers](testing.md) is the report format this page reads.
- [Undefined Behaviour](../c/undefined-behavior.md) explains why some crashes have no site at all.
- [Error Handling](../c/error-handling.md) is the C-side habit that prevents the null destination.
