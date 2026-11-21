/* LAB 06-02  structs_padding/02_union_typing.c
 * Drills: a union overlays storage; a tag is what makes the overlay safe.
 * Build:  cmake --build build --target c_06_union
 */
#include "craft_assert.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Untagged: nothing records which member was last written, so reading the
 * wrong one is a type pun that compiles. */
union Untagged {
  int32_t i;
  float f;
  char bytes[4];
};

/* Tagged: the enum and the value are one object, and the reader must switch
 * on the tag before touching the value. */
enum Kind { KIND_INT, KIND_FLOAT, KIND_TEXT };

struct Tagged {
  enum Kind kind;
  union {
    int32_t i;
    float f;
    char text[16];
  } as;
};

static struct Tagged make_int(int32_t v) {
  struct Tagged t;
  t.kind = KIND_INT;
  t.as.i = v;
  return t;
}

static struct Tagged make_float(float v) {
  struct Tagged t;
  t.kind = KIND_FLOAT;
  t.as.f = v;
  return t;
}

static void print_tagged(const struct Tagged *t) {
  switch (t->kind) {
  case KIND_INT:
    printf("int    %d\n", t->as.i);
    break;
  case KIND_FLOAT:
    printf("float  %.3f\n", (double)t->as.f);
    break;
  case KIND_TEXT:
    printf("text   %s\n", t->as.text);
    break;
  }
}

int main(void) {
  union Untagged u;
  u.i = 0x3F800000; /* the bit pattern of 1.0f */
  CRAFT_ASSERT(u.f == 1.0f); /* the pun works here only because it was chosen */
  printf("untagged: 0x3F800000 read as float = %.1f\n", (double)u.f);

  struct Tagged a = make_int(42);
  struct Tagged b = make_float(2.5f);
  print_tagged(&a);
  print_tagged(&b);

  CRAFT_ASSERT(a.kind == KIND_INT);
  CRAFT_ASSERT(b.kind == KIND_FLOAT);

  /* sizeof the union is the largest member, not the sum; sizeof the tagged
   * struct is the enum plus that union, with padding for alignment. */
  printf("sizeof(union)  = %zu\n", sizeof(union Untagged));
  printf("sizeof(tagged) = %zu\n", sizeof(struct Tagged));
  return 0;
}
