/* LAB 15-01  linked_structures/01_list_reverse.c
 * Drills: reversing a singly linked list in place, the three-pointer dance, and
 *         the free walk that must follow the same shape.
 * Build:  cmake --build build --target c_15_list
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
  int value;
  struct node *next;
} node;

static node *push(node *head, int value) {
  node *n = (node *)malloc(sizeof *n);
  if (n == NULL) {
    return NULL; /* the caller checks: a failed push must not lose the list */
  }
  n->value = value;
  n->next = head;
  return n;
}

/* In place, O(n) time and O(1) space: prev, cur, next, one link rewritten at a
 * time. The alternative — a new list built by pushing each element — is O(n)
 * extra memory for the same result. */
static node *reverse(node *head) {
  node *prev = NULL;
  node *cur = head;
  while (cur != NULL) {
    node *next = cur->next; /* save before overwrite: the one required step */
    cur->next = prev;
    prev = cur;
    cur = next;
  }
  return prev;
}

static size_t length(const node *head) {
  size_t n = 0;
  for (const node *p = head; p != NULL; p = p->next) {
    ++n;
  }
  return n;
}

static void free_all(node *head) {
  while (head != NULL) {
    node *next = head->next;
    free(head);
    head = next;
  }
}

int main(void) {
  node *head = NULL;
  for (int i = 1; i <= 5; ++i) {
    head = push(head, i);
    CRAFT_ASSERT(head != NULL);
  }

  CRAFT_ASSERT(length(head) == 5);
  CRAFT_ASSERT(head->value == 5); /* push puts the newest first */
  printf("built: 5 4 3 2 1\n");

  head = reverse(head);
  CRAFT_ASSERT(head->value == 1);
  CRAFT_ASSERT(head->next->value == 2);
  CRAFT_ASSERT(length(head) == 5);
  printf("reversed: 1 2 3 4 5\n");

  /* Reversing twice is the identity. Two calls on the already-reversed list
   * return it to 1 2 3 4 5, and a third would flip it again — which is why the
   * check counts the operations rather than assuming an even number. */
  head = reverse(head);
  CRAFT_ASSERT(head->value == 5);
  CRAFT_ASSERT(head->next->value == 4);
  printf("reversed twice back to 5 4 3 2 1\n");

  free_all(head);
  printf("freed: the walk follows next, saved before free\n");
  return 0;
}
