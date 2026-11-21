# Strings

A C string has no length. That is the whole chapter, and every section is a consequence: `strlen` walks, a loop that calls it becomes quadratic, `strncpy` copies a fixed count rather than a string, and a byte is not a character. I put the measurement first because the quadratic behaviour is invisible until you count the bytes read, and I keep the Unicode note short because the fix is a library, not a habit. The Field Note is a comparison that inverted a state machine, which is what happens when a string API returns a number you did not read.

A C string is not a type. It is a convention: an array of `char`, terminated by a zero byte, with the understanding that functions stop at the first zero they meet. Every defect in this chapter follows from one of those three clauses being violated — usually the third.

## 1. The convention, measured

```c
char s[] = "abc";
printf("%zu %zu\n", sizeof s, strlen(s));   /* 4 3 */
```

`sizeof` is 4 because the array contains three characters and the terminator. `strlen` is 3 because it counts until it finds the terminator. The two numbers differ by one, and that difference is the reason `sizeof` is safe and `strlen` is a walk.

`'\0'` is a byte with value zero, not the character `'0'`, which has value 48. Storing `'0'` where a terminator was intended produces a string that continues into whatever memory follows, and the defect surfaces when a later function reads past the intended end.

Reimplementing `strlen` is the shortest way to see that the function has no other information to work with:

```c
/* LAB 05-01  strings/01_strlen_reimpl.c */
#include <stdio.h>
#include <stddef.h>

static size_t length_of(const char *s) {
    const char *p = s;
    while (*p != '\0') {
        ++p;
    }
    return (size_t)(p - s);
}

int main(void) {
    const char *text = "membrane";
    printf("%zu\n", length_of(text));   /* 8 */
    return 0;
}
```

There is no length stored anywhere. The only way to know where the string ends is to walk it, which is why `strlen` is linear and why calling it inside a loop condition turns a linear algorithm into a quadratic one:

```c
for (int i = 0; i < strlen(s); ++i) { }   /* O(n^2) */
for (size_t i = 0; s[i] != '\0'; ++i) { } /* O(n)   */
```

The second form is not a micro-optimisation. On a 10,000-character sequence the first form performs about 50 million byte reads to do 10,000 iterations of work, and the difference is measurable in the profiling chapter.

## 2. Reading input without `gets`

`gets` was removed from C11 for a reason that is easy to demonstrate and impossible to fix: it has no length parameter, so it cannot know where the buffer ends.

```c
char buf[8];
gets(buf);       /* not in C11; reads until newline regardless of size */
```

`fgets` is the replacement, and it has two behaviours that need checking rather than assuming:

```c
/* LAB 05-02  strings/02_safe_input.c */
char line[64];
if (fgets(line, sizeof line, stdin) == NULL) {
    return 1;                 /* EOF or error, not an empty line */
}

size_t len = strlen(line);
if (len > 0 && line[len - 1] == '\n') {
    line[len - 1] = '\0';     /* strip the newline, if it is there */
}
```

The first check is easy to forget: `fgets` returns `NULL` at end of input, and treating that as an empty string loses the distinction. The second check matters because `fgets` includes the newline when it fits, and a comparison like `strcmp(line, "yes")` then fails on input that looks identical on screen.

When the line does not fit, `fgets` returns the truncated part and leaves the rest in the stream. The next read returns the remainder, which usually appears as a mysterious extra iteration. The reliable pattern is to check for the newline and, if it is missing, consume the rest of the line before continuing:

```c
if (len > 0 && line[len - 1] != '\n') {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}
```

## 3. `strncpy` is not the bounded `strcpy`

```c
char dst[8];
strncpy(dst, "abcdefgh", sizeof dst);   /* fills 8 bytes, no terminator */
```

`strncpy` copies at most `n` characters and stops early only if it finds a terminator in the source. When the source is longer than `n`, the result has no terminator at all, and every subsequent string function walks off the end. The function was designed for fixed-width records in old file formats, not for C strings.

The two usable alternatives are explicit:

```c
snprintf(dst, sizeof dst, "%s", src);   /* always terminates, truncates silently */

size_t n = strlen(src) < sizeof dst - 1 ? strlen(src) : sizeof dst - 1;
memcpy(dst, src, n);
dst[n] = '\0';
```

`snprintf` is the one to reach for by default. Its cost is that truncation is silent, so a caller who needs to detect it must compare the return value against the buffer size.

## 4. A character is not a byte, and a byte is not a glyph

`char` holds one byte. UTF-8 encodes one character in one to four bytes, which means `strlen` on a UTF-8 string counts bytes, not characters, and `s[i]` indexes bytes, not characters. For English text the two coincide often enough to hide the problem; for a protein sequence they always coincide, since the alphabet is ASCII; for a name with a non-Latin character they diverge immediately.

The biology section relies on the ASCII case and states the assumption at the top of each parser. Text handling in general does not, and the fix is a library, not a loop.

## 5. Field note: the comparison that inverted a state machine

The original `sumWord.c` counted words by tracking whether the previous character was inside a word. The loop condition contained one character that inverted the logic:

```c
while (str[j] != ' ' && str[j] == '\0') {   /* the original */
```

The second test should have been `!=`. As written, the loop continues only when the character is simultaneously not a space and the terminator — a condition that is never true, so the body never executes and the word count stays at zero for every input.

The corrected condition is `str[j] != ' ' && str[j] != '\0'`. The interesting part is not the fix but the diagnosis: the function compiled, ran, and returned a plausible answer (zero words) for an input with no words. A test with one word would have caught it; a test with an empty string would not.

The general lesson belongs here rather than in a debugger chapter: string code is a state machine, and a state machine with an inverted transition produces output that is internally consistent and externally wrong.

## Continue

- [Structs and Padding](structs-and-padding.md) explains why `char name[20]` inside a struct may cost more than 20 bytes.
- [Files and Streams](files-and-streams.md) applies `fgets` to file input, where truncation is the normal case.
- [FASTA](../bio/fasta.md) parses sequence records, the one place where the ASCII assumption is safe and worth stating.
