/* LAB 02-01  stack_frames/01_frame_addresses.c
 * Drills: addresses of locals, the direction the stack grows, frame reuse.
 * Build:  cmake --build build --target c_02_frames
 */
#include <stdint.h>
#include <stdio.h>

/* Returns an address as an integer so it can be printed without a cast in
 * printf. The value is only meaningful within the call that produced it. */
static uintptr_t addr_of(const void *p) { return (uintptr_t)p; }

static void frame_probe(int depth) {
  int local = depth;
  printf("depth %d  local=%-3d addr=%p\n", depth, local, (void *)&local);
  if (depth < 3) {
    frame_probe(depth + 1);
  }
}

static int add(int a, int b);

int main(void) {
  int outer = 1;
  int inner = 2;

  printf("outer  %p\n", (void *)&outer);
  printf("inner  %p\n", (void *)&inner);
  printf("lower address is %s declared later\n",
         addr_of(&inner) < addr_of(&outer) ? "the" : "not");

  /* Two calls to the same function reuse the same frame: the address printed
   * by the second call equals the first. That reuse is why a pointer to a
   * local is invalid the moment the call returns. */
  printf("\nrecursion:\n");
  frame_probe(0);

  /* main's own frame is reused by nothing here, so its locals keep their
   * addresses; the callee's frame sits below it and is gone after return. */
  printf("\nmain's frame still holds outer=%d at %p\n", outer,
         (void *)&outer);
  printf("add(2, 3) = %d\n", add(2, 3));
  return 0;
}

static int add(int a, int b) { return a + b; }
