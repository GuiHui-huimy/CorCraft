# Linked Structures

A linked list is two fields and one invariant, and the invariant is the whole difficulty. I begin with the node and the two operations that matter, then reversal with the invariant written down, because the three-pointer dance is only confusing when the invariant is implicit. Teardown follows the same shape, which is the point; then cycles and the two-speed walk; then a ring buffer, which is the version of the structure that does not allocate per element. The Field Note is a free that broke the list by freeing the wrong node.

A linked list is the smallest structure whose shape is not implied by its type. The nodes are separate heap blocks, the links are pointers, and every operation is a small exercise in ownership. It is also the standard interview question, which means most readers have memorised the reversal loop without ever seeing why it works.

## 1. The node, and the two operations that matter

```c
struct node {
    int          value;
    struct node *next;
};
```

Insertion at the front is three lines and cannot fail if the allocation succeeded:

```c
/* LAB 15-01  linked_structures/01_list_reverse.c */
static struct node *push_front(struct node *head, int value) {
    struct node *n = malloc(sizeof *n);
    if (n == NULL) { return head; }        /* caller checks for no change */
    n->value = value;
    n->next = head;
    return n;
}
```

The function returns the new head rather than modifying the caller's variable, because C has no references. The caller must assign: `head = push_front(head, 3);`. A caller that forgets the assignment loses the new node, which then leaks — the pointer to it existed only inside the function.

Insertion at the back requires walking to the end, which is O(n) per insertion unless the list keeps a tail pointer. That is the first design decision this chapter makes: a `struct list { struct node *head, *tail; }` costs eight bytes and turns `push_back` into O(1).

## 2. Reversal, with the invariant made visible

```c
static struct node *reverse(struct node *head) {
    struct node *prev = NULL;
    struct node *cur  = head;
    while (cur != NULL) {
        struct node *next = cur->next;   /* save before overwriting */
        cur->next = prev;                /* reverse the link */
        prev = cur;                      /* advance prev */
        cur  = next;                     /* advance cur */
    }
    return prev;
}
```

Three pointers, four assignments, and one invariant: at the top of each iteration, `prev` is the fully reversed prefix and `cur` is the untouched remainder. The `next` save is not optional; without it, `cur->next = prev` destroys the only pointer to the rest of the list, and the loop walks into the reversed part, producing a cycle.

The reference program asserts on a five-node list: it pushes 1 to 5, checks the head is 5, reverses once and checks the head is 1 with a length of 5, then reverses again and checks 5 4. The one-node list is the case it does not cover, and it is where implementations that return `head` instead of `prev` fail, because after the loop `head` still points at the node whose `next` is now `NULL` — which is correct for one node and wrong for any longer list. Adding that case is the first thing to do after copying the file.

## 3. Teardown, in the right order

```c
static void free_list(struct node *head) {
    while (head != NULL) {
        struct node *next = head->next;
        free(head);
        head = next;
    }
}
```

The saved `next` is the same idea as in the reversal loop: freeing `head` invalidates `head->next`, so the next pointer must be copied first. A teardown that writes `free(head); head = head->next;` reads freed memory, which AddressSanitizer reports as a heap-use-after-free at the second line.

Recursive teardown is also possible and is a trap for long lists: a 100,000-node list recurses 100,000 times and overflows the stack, which is the failure from chapter 2 applied to a data structure.

## 4. Cycles, and the two-speed walk

A list that accidentally points back into itself has no end, so every traversal loops forever. Detecting it without extra memory uses two pointers advancing at different speeds:

```c
static int has_cycle(const struct node *head) {
    const struct node *slow = head;
    const struct node *fast = head;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) { return 1; }
    }
    return 0;
}
```

If there is a cycle, the fast pointer enters it first and the slow pointer eventually catches up from behind; if there is none, the fast pointer reaches `NULL`. The proof is short and worth reading once, because the technique generalises to any "detect repetition in a finite state space" problem.

The cycle itself is usually created by the reversal defect above, or by a node inserted twice. Both are ownership errors: a node with two predecessors is a node that will be freed twice or traversed twice.

## 5. A ring buffer, without allocation per element

A different shape worth knowing before the C++ containers: a fixed-size array with a head index and a count, wrapping by modulo.

```c
/* LAB 15-02  linked_structures/02_ring_buffer.c */
struct ring {
    int   data[8];
    size_t head;
    size_t count;
};

static int ring_push(struct ring *r, int v) {
    if (r->count == 8) { return -1; }               /* full */
    r->data[(r->head + r->count) % 8] = v;
    ++r->count;
    return 0;
}

static int ring_pop(struct ring *r, int *out) {
    if (r->count == 0) { return -1; }               /* empty */
    *out = r->data[r->head];
    r->head = (r->head + 1) % 8;
    --r->count;
    return 0;
}
```

The modulo is the only subtle part. Writing `(head + count) % 8` rather than `head + count` avoids growing the index without bound, and the count distinguishes full from empty — without it, `head == tail` means both, which is the classic ambiguity in a naive ring.

The ring buffer is the right structure when the size is bounded and known, which is the case for the sliding windows in the biology section: a contact-map builder keeps the last `k` residues in a ring rather than allocating a deque.

## 6. Field note: the free that broke the list

An early implementation freed the list in a loop that read the next pointer after freeing the current node:

```c
while (p != NULL) {
    free(p);
    p = p->next;        /* reads freed memory */
}
```

On the developer's machine this usually worked, because the allocator had not yet reused the block and the first eight bytes still held the old `next` value. Under AddressSanitizer it failed immediately; under a different allocator it corrupted a random node and crashed later.

The repair is the two-line form above. The lesson is broader than the fix: a use-after-free that happens to work is more dangerous than one that crashes, because it survives long enough to be copied into other code.

## Continue

- [Error Handling](error-handling.md) gives `push_front`'s failure path a contract.
- [RAII and Smart Pointers](../cpp/raii.md) replaces manual teardown with a destructor.
- [STL Containers](../cpp/containers.md) shows the same operations with iterators and no visible links.
