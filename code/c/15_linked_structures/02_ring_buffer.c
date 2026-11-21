/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 15-02  linked_structures/02_ring_buffer.c
 * Drills: a fixed-size circular buffer, the full/empty distinction, and the
 *         index arithmetic that wraps without a branch.
 * Build:  cmake --build build --target c_15_ring
 */
#include "craft_assert.h"

#include <stdio.h>

#define CAP 8

typedef struct {
  int data[CAP];
  size_t head;  /* next read */
  size_t tail;  /* next write */
  size_t count; /* the field that makes full and empty distinguishable */
} ring;

static void ring_init(ring *r) {
  r->head = 0;
  r->tail = 0;
  r->count = 0;
}

static int ring_full(const ring *r) { return r->count == CAP; }
static int ring_empty(const ring *r) { return r->count == 0; }

static int ring_push(ring *r, int value) {
  if (ring_full(r)) {
    return -1; /* full is a reportable condition, not an assert */
  }
  r->data[r->tail] = value;
  r->tail = (r->tail + 1) % CAP;
  ++r->count;
  return 0;
}

static int ring_pop(ring *r, int *out) {
  if (ring_empty(r)) {
    return -1;
  }
  *out = r->data[r->head];
  r->head = (r->head + 1) % CAP;
  --r->count;
  return 0;
}

int main(void) {
  ring r;
  ring_init(&r);

  /* The defect this prevents: using head == tail for both full and empty.
   * With no count field the two states are the same state. */
  CRAFT_ASSERT(ring_empty(&r));
  CRAFT_ASSERT(!ring_full(&r));

  for (int i = 0; i < CAP; ++i) {
    CRAFT_ASSERT(ring_push(&r, i) == 0);
  }
  CRAFT_ASSERT(ring_full(&r));
  CRAFT_ASSERT(ring_push(&r, 99) == -1); /* full: refused, nothing overwritten */
  printf("after %d pushes: full=%d count=%zu\n", CAP, ring_full(&r), r.count);

  int v;
  for (int i = 0; i < CAP; ++i) {
    CRAFT_ASSERT(ring_pop(&r, &v) == 0);
    CRAFT_ASSERT(v == i); /* FIFO order preserved across the wrap */
  }
  CRAFT_ASSERT(ring_empty(&r));
  printf("popped in order 0..%d, buffer empty again\n", CAP - 1);

  /* Wrap: push and pop repeatedly so head and tail cross the boundary many
   * times; the modulo arithmetic is what makes that work. */
  for (int i = 0; i < 100; ++i) {
    CRAFT_ASSERT(ring_push(&r, i) == 0);
    if (i >= 3) {
      CRAFT_ASSERT(ring_pop(&r, &v) == 0);
      CRAFT_ASSERT(v == i - 3);
    }
  }
  printf("100 push/pop cycles across the wrap: exact\n");
  return 0;
}
