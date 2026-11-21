/* LAB 07-02  heap/02_double_free.c
 * Drills: the second free, and why setting the pointer to NULL is the guard.
 * Build:  cmake --build build --target c_07_double
 *
 * Default mode is correct: free once, then set NULL, and a second free is a
 * no-op. --unsafe performs the second free for the sanitizer to report.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *make_buffer(size_t n) {
  char *p = (char *)malloc(n);
  if (p == NULL) {
    return NULL;
  }
  memset(p, 'x', n);
  p[n - 1] = '\0';
  return p;
}

int main(int argc, char **argv) {
  int unsafe = (argc > 1 && strcmp(argv[1], "--unsafe") == 0);

  char *p = make_buffer(16);
  CRAFT_ASSERT(p != NULL);
  printf("allocated 16 bytes at %p, last char '%c'\n", (void *)p, p[14]);

  free(p);
  if (!unsafe) {
    p = NULL; /* the guard: after this, free(p) is free(NULL), which is defined */
    free(p);
    printf("second free was free(NULL): defined behaviour\n");
  } else {
    /* p is dangling at this point. The second free is undefined behaviour and
     * the allocator's double-free detector is what catches it. */
    printf("performing the second free (expect an ASan report)\n");
    free(p);
  }
  return 0;
}
