# Files and Streams

I wrote a program that lost its last line for a month, and this chapter is the explanation. Open modes come first because the wrong one truncates the file, then line reading without the `feof` trap, then block reads, then seeking and what it does to the buffer. Buffering gets its own section because a write that never reached the disk is the failure that survives a successful exit, and the Field Note is exactly that truncation.

A `FILE *` is a buffered handle, not a file descriptor and not a path. Almost every file defect comes from confusing the three: assuming a read reached the disk, assuming a write failed visibly, or assuming the end of the file is detectable before the next read.

## 1. Open modes, and the one that surprises

| Mode | Read | Write | If exists | If missing |
| --- | --- | --- | --- | --- |
| `"r"` | yes | no | opens | fails |
| `"w"` | no | yes | truncates | creates |
| `"a"` | no | append | appends | creates |
| `"r+"` | yes | yes | opens | fails |
| `"rb"`, `"wb"` | binary variants | — | — | — |

`"w"` truncates on open, before a single byte is written. A program that opens a file for writing, fails partway through, and exits leaves a truncated file and no indication that the original content is gone. The defensive pattern is to write to a temporary name and rename on success, which is what the FASTA writer in the biology section does.

`"b"` has no effect on POSIX systems; on Windows it suppresses the CRLF translation. The book always uses `"rb"` and `"wb"` explicitly, because a PDB file read in text mode on Windows has its line endings rewritten and its column offsets shifted by nothing — which is correct — while a binary trajectory read in text mode is corrupted.

## 2. Reading lines without the `feof` trap

```c
/* LAB 12-01  files_streams/01_line_reader.c */
#include <stdio.h>

int main(void) {
    FILE *fp = fopen("sample.txt", "r");
    if (fp == NULL) { return 1; }

    char line[256];
    int count = 0;
    while (fgets(line, sizeof line, fp) != NULL) {
        ++count;
    }
    printf("lines: %d\n", count);
    fclose(fp);
    return 0;
}
```

The loop condition tests the return value of `fgets`, not `feof(fp)`. The difference is one extra iteration: `feof` only becomes true *after* a read has failed, so `while (!feof(fp))` always processes a final empty line. The condition is the classic C file bug, and it is invisible unless the file has a trailing newline or the processing has side effects.

Why the extra iteration happens: `fgets` reads until the newline, returns the string, and the stream still has not observed EOF. The next call reads zero bytes, sets the EOF flag, and returns `NULL` — but by then the loop has already run once with stale data if the condition was `feof`.

Truncation is the other case to handle. When a line is longer than the buffer, `fgets` returns the first part without a newline. Detecting it is the same check as before:

```c
size_t len = strlen(line);
if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
    /* the line was truncated: consume or report */
}
```

## 3. Reading a block

For binary data, `fread` is the primitive and its return value is a count of items, not bytes:

```c
double coords[3];
size_t got = fread(coords, sizeof coords[0], 3, fp);
if (got != 3) { /* short read: file ended or an error occurred */ }
```

A short read is not necessarily an error. If the file legitimately ends mid-record, `got` is smaller and the caller must decide whether that is a valid truncation or corruption. Checking `ferror(fp)` distinguishes the two: a nonzero result means the stream itself failed, while `feof(fp)` means the data simply ran out. The habit is to check both after a short read, not just the count.

## 4. Seeking, and what it does to the buffer

`fseek` and `ftell` manipulate a position measured in bytes, and both flush or discard buffered data as needed. Three rules keep them predictable:

```c
fseek(fp, 0, SEEK_SET);            /* rewind: next read starts at byte 0 */
long pos = ftell(fp);              /* the current byte offset */
fseek(fp, 20L, SEEK_CUR);          /* advance 20 bytes from here */
```

The offset type is `long`, which is 4 bytes on Windows even in 64-bit builds; a file over 2 GB cannot be addressed portably with `ftell`. `fgetpos`/`fsetpos` use an opaque type and are the portable alternative. For the coordinate files in this book, which are under 100 MB, `long` is adequate and the note is a footnote rather than a design constraint.

Seeking in a file opened for writing is more delicate: the standard requires a flush between a write and a seek, and `fseek` does that implicitly. Mixing `fread` and `fwrite` on the same handle without an intervening seek is undefined.

## 5. Buffering, and the write that never reached the disk

`fwrite` returns when the data is in the stream's buffer, not on the disk. A program that writes, then calls `_exit`, loses the buffer. `fclose` flushes, and `fflush` flushes explicitly.

```c
if (fwrite(buf, 1, len, fp) != len) { /* handle */ }
if (fflush(fp) != 0) { /* the flush failed: the write is not durable */ }
if (fclose(fp) != 0) { /* the final flush failed */ }
```

`fclose` returning nonzero is the case people skip, and it is the one that matters for durability: the data may have been buffered successfully and the final write may still have failed. A writer that ignores the return value reports success for a file that was never written.

The cost of buffering is why the line reader above is fast: it performs one `read` system call per buffer, not per line. It is also why a program that reads a file, writes to another, and never closes either can produce a target file with content that appears only when the process exits.

## 6. Error checking, at the right moment

Every stream function can fail, and the failure is reported at the point of the call, not at the point where the data is used. The pattern that works is to check the return of each call in the direction of data flow:

```c
FILE *fp = fopen(path, "rb");
if (fp == NULL) { perror(path); return -1; }

char *buf = craft_read_file(path, &len);   /* helpers in code/include */
...
int failed = (fclose(fp) != 0);
return failed ? -1 : 0;
```

`perror` prints the reason from `errno` with the caller's string as a prefix, which is the least-effort diagnostic and the one to use before a real logging system exists. Chapter 16 turns this into a return-code contract.

## 7. Field note: the truncation nobody noticed

A parser wrote its result with `"w"` and a single `fwrite`, then checked the return value of the write. The write succeeded; the flush inside `fclose` failed because the disk was full. The program exited zero, and the output file was shorter than expected by exactly the amount that did not fit.

The fix had two parts: check `fclose`, and write to a temporary file then rename. The second part is what makes the failure atomic — a reader either sees the previous complete file or the new complete file, never a truncated one.

The general lesson belongs to this chapter rather than to error handling: a flushed write is a claim about durability, and only the last call in the chain can make that claim.

## Continue

- [Translation Units](translation-units.md) covers what happens when several files declare the same stream helper.
- [Error Handling](error-handling.md) formalises the return-code contract used above.
- [FASTA](../bio/fasta.md) applies the line reader to a format whose records span two lines.
