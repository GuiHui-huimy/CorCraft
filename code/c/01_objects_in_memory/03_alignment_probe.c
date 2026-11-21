/* LAB 01-03  objects_in_memory/03_alignment_probe.c
 * Drills: alignof, offsetof, and the padding those two numbers explain.
 * Build:  cmake --build build --target c_01_align
 */
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

struct Mixed {
  char c;
  int i;
  char d;
  double x;
};

struct Rearranged {
  double x;
  int i;
  char c;
  char d;
};

int main(void) {
  printf("alignof(char)   %zu\n", alignof(char));
  printf("alignof(int)    %zu\n", alignof(int));
  printf("alignof(double) %zu\n", alignof(double));

  printf("\nstruct Mixed { char; int; char; double; }\n");
  printf("  sizeof          %zu\n", sizeof(struct Mixed));
  printf("  offsetof(c)     %zu\n", offsetof(struct Mixed, c));
  printf("  offsetof(i)     %zu   (3 bytes padded after c)\n",
         offsetof(struct Mixed, i));
  printf("  offsetof(d)     %zu\n", offsetof(struct Mixed, d));
  printf("  offsetof(x)     %zu   (4 bytes padded after d)\n",
         offsetof(struct Mixed, x));

  printf("\nstruct Rearranged { double; int; char; char; }\n");
  printf("  sizeof          %zu\n", sizeof(struct Rearranged));
  printf("  offsetof(x)     %zu\n", offsetof(struct Rearranged, x));
  printf("  offsetof(i)     %zu\n", offsetof(struct Rearranged, i));

  /* The claim the chapter makes: order the members by decreasing alignment and
   * the padding mostly disappears. */
  printf("\nsaved by reordering: %zu bytes per object\n",
         sizeof(struct Mixed) - sizeof(struct Rearranged));
  return 0;
}
