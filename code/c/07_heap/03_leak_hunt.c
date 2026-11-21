/* LAB 07-03  heap/03_leak_hunt.c
 * Drills: one allocation, one owner, one free — and what the leak detector
 *         reports when an owner is lost.
 * Build:  cmake --build build --target c_07_leak
 *
 * Default mode frees everything. --leak drops the pointer before freeing, so
 * LeakSanitizer has something to report.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *name;
  char *sequence;
  size_t length;
} record;

static record *record_new(const char *name, size_t length) {
  record *r = (record *)malloc(sizeof *r);
  if (r == NULL) {
    return NULL;
  }
  r->name = (char *)malloc(strlen(name) + 1);
  r->sequence = (char *)malloc(length + 1);
  if (r->name == NULL || r->sequence == NULL) {
    free(r->name);
    free(r->sequence);
    free(r);
    return NULL;
  }
  memcpy(r->name, name, strlen(name) + 1);
  memset(r->sequence, 'A', length);
  r->sequence[length] = '\0';
  r->length = length;
  return r;
}

static void record_free(record *r) {
  if (r == NULL) {
    return;
  }
  free(r->name);
  free(r->sequence);
  free(r);
}

int main(int argc, char **argv) {
  int leak = (argc > 1 && strcmp(argv[1], "--leak") == 0);

  record *r = record_new("ubiquitin", 76);
  CRAFT_ASSERT(r != NULL);
  CRAFT_ASSERT(r->length == 76);
  printf("record \"%s\" length %zu at %p\n", r->name, r->length, (void *)r);

  if (leak) {
    /* The owner is lost: the three blocks are still allocated and nothing
     * holds their addresses. This is what "definitely lost" means. */
    r = NULL;
    printf("dropped the only pointer (expect a leak report)\n");
  } else {
    record_free(r);
    printf("freed all three blocks: one owner, three frees\n");
  }
  return 0;
}
