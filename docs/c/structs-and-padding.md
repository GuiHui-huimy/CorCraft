# Structs and Padding

I learned padding the expensive way: a struct written to a file on one compiler and read on another, with two bytes of garbage between the fields. This page starts from `offsetof` rather than from a diagram, because an offset is a number you can print, then states the placement rule, then reorders the members to show what changes, and then treats padding as part of the object rather than as wasted space. Unions, bit-fields and flexible array members come last, each with the portability limit that makes it a deliberate choice.

A struct is a layout, and the layout is not the sum of its fields. This chapter measures the gaps, explains where they come from, and shows the two cases where the gaps are load-bearing: serialization and comparison.

## 1. Offsets, not guesses

```c
/* LAB 06-01  structs_padding/01_layout_table.c */
#include <stdio.h>
#include <stddef.h>

struct sample {
    char   tag;      /* 1 byte  */
    int    id;       /* 4 bytes */
    char   flag;     /* 1 byte  */
    double value;    /* 8 bytes */
};

int main(void) {
    printf("sizeof      %zu\n", sizeof(struct sample));
    printf("offset tag  %zu\n", offsetof(struct sample, tag));
    printf("offset id   %zu\n", offsetof(struct sample, id));
    printf("offset flag %zu\n", offsetof(struct sample, flag));
    printf("offset val  %zu\n", offsetof(struct sample, value));
    return 0;
}
```

On x86-64 Linux the output is:

```
sizeof      24
offset tag  0
offset id   4
offset flag 8
offset val  16
```

The fields total 14 bytes; the object is 24. The three missing bytes after `tag` exist so that `id` starts at a multiple of 4. The seven bytes after `flag` exist so that `value` starts at a multiple of 8. And the object itself is padded to 24 because its alignment is the maximum of its members' alignments, 8 — an array of these structs must keep every `value` aligned, so the stride must be a multiple of 8.

## 2. The placement rule

The rule the compiler follows is short enough to apply by hand:

1. Each member is placed at the next offset that is a multiple of its own alignment.
2. The struct's alignment is the maximum alignment among its members.
3. The struct's size is rounded up to a multiple of its alignment.

Applying it to the struct above: `tag` at 0 (size 1), `id` needs a multiple of 4 so it goes to 4 (occupying 4–7), `flag` at 8, `value` needs a multiple of 8 so it goes to 16 (occupying 16–23), total 24.

## 3. Reordering is free, and sometimes necessary

The same fields in a different order produce a smaller object:

```c
struct packed_order {
    double value;    /* 0  */
    int    id;       /* 8  */
    char   tag;      /* 12 */
    char   flag;     /* 13 */
};                   /* sizeof == 16 */
```

Sixteen bytes instead of twenty-four, with no behavioural change and no `#pragma pack`. The cost is that the declaration no longer reads in a logical order, which is a real cost: a struct whose fields follow the record they model is easier to audit than one arranged for density. Reorder when the object is stored by the million, not by the dozen.

`#pragma pack(1)` also removes padding, and it should be treated as a last resort. On x86-64 it produces unaligned accesses, which are slow rather than fatal; on some ARM configurations they fault. The portable way to serialize a struct is not to pack it but to write each field explicitly, which is what chapter 12 does with PDB records.

## 4. Padding is part of the object

Two consequences that surprise people:

```c
struct sample a, b;
memset(&a, 0, sizeof a);
b = a;
if (memcmp(&a, &b, sizeof a) != 0) { /* may be true: padding is copied too */ }
```

`memcmp` compares padding bytes as well, and they are not guaranteed to be equal. Assignment copies them, so this usually works, but a struct that has been `memset` and a struct that was initialised field by field may differ in the gaps. Comparing structs byte-wise is a defect waiting for a different compiler version.

The second consequence is that `memset` on a struct is legitimate, precisely because padding is part of the object: the standard permits it, and the result is a struct with every byte, including the gaps, set to zero.

## 5. Unions: one region, several readings

A union allocates enough storage for its largest member, and every member starts at offset 0. Writing one member and reading another is how representation probes work — the endianness probe in chapter 1 is exactly this.

```c
/* LAB 06-02  structs_padding/02_union_typing.c */
union value {
    uint32_t bits;
    float    as_float;
};

union value v = { .bits = 0x40490FDBu };
printf("%.6f\n", v.as_float);   /* 3.141593 */
```

This is defined behaviour in C: the union's members share storage, and reading a different member than the last written is permitted (the result is unspecified only if the member read is a trap representation). It is the one sanctioned way to reinterpret bits without violating aliasing rules — the next chapter explains why the pointer cast that does the same thing is not sanctioned.

A tagged union — a `union` plus an `enum` recording which member is live — is how C models a variant. The `enum` is not optional; without it, nothing tells the reader which member holds a value.

## 6. Bit-fields, and their portability limit

```c
struct flags {
    unsigned ready : 1;
    unsigned mode  : 3;
    unsigned       : 4;   /* unnamed padding */
};
```

Bit-fields pack several small values into one storage unit, which is useful for hardware registers and occasionally for dense flags. The portability limit is real: the standard does not specify the order of allocation within a unit, the behaviour at a unit boundary, or whether an `int` bit-field may straddle one. A struct of bit-fields serialized on GCC and read on MSVC can produce different values from identical bytes.

For anything that crosses a machine boundary, use explicit masks on a `uint32_t` — the technique in the bit-manipulation chapter — and treat bit-fields as an internal convenience only.

## 7. Flexible array members

A struct whose last member is an incomplete array is the standard way to allocate a header and a variable-length body in one block:

```c
struct sequence {
    size_t length;
    char   bases[];      /* C99 flexible array member */
};

struct sequence *make(size_t n) {
    struct sequence *s = malloc(sizeof *s + n + 1);
    if (s != NULL) {
        s->length = n;
    }
    return s;
}
```

`sizeof(struct sequence)` excludes the body, and the allocation adds `n + 1` bytes for the bases and the terminator. One `malloc`, one `free`, and the body is adjacent to its header — which is what makes this the preferred shape for parsed records, including the FASTA records in the biology section.

## 8. Field note: the `sizeof` that measured a pointer

The original `mocal.c` allocated ten strings in two levels and then freed only the outer array:

```c
str = (char **)malloc(10 * sizeof(char *));
for (int i = 0; i < 10; ++i) {
    str[i] = (char *)malloc(20 * sizeof(char *));   /* sizeof a pointer, not a char */
    ...
}
...
free(str);                                          /* ten blocks left behind */
```

Two defects in four lines. The inner allocation uses `sizeof(char *)`, which happens to equal the intended `sizeof(char)` on this platform only because both are 8 and 1 respectively — it over-allocates by a factor of 8, harmlessly here and catastrophically in a large array. And the teardown frees the outer block while the ten inner blocks remain allocated, so every call leaks 160 bytes.

The corrected teardown is a loop, and the corrected allocation is `sizeof *str[i]`:

```c
for (int i = 0; i < 10; ++i) { free(str[i]); }
free(str);
```

The idiom `sizeof *p` rather than `sizeof(type)` removes the entire class of defect: it cannot disagree with the type of the pointer it is sizing.

## Continue

- [The Heap](heap.md) picks up the ownership question the field note left open.
- [Qualifiers and Aliasing](qualifiers-and-aliasing.md) explains why reading a union is legal and casting a pointer is not.
- [PDB Parsing](../bio/pdb-format.md) is the real-world version of this chapter: fixed-width records, explicit fields, no packing.
