/* LAB 06-01  structs_padding/01_layout_table.c
 * Drills: one data set in four member orders, the size each costs, and the
 *         bytes that are padding rather than data.
 * Build:  cmake --build build --target c_06_layout
 */
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

struct ByAlignment { /* double first, then int, then the bytes */
  double value;
  int id;
  char flag;
  char tag;
};

struct Mixed {
  char flag;
  double value;
  char tag;
  int id;
};

struct Worst {
  char a;
  double b;
  char c;
  double d;
};

int main(void) {
  printf("struct ByAlignment  sizeof %2zu  (8 + 4 + 1 + 1, 2 padded)\n",
         sizeof(struct ByAlignment));
  printf("struct Mixed        sizeof %2zu  (1 + 7 pad + 8 + 1 + 3 pad + 4)\n",
         sizeof(struct Mixed));
  printf("struct Worst        sizeof %2zu  (1 + 7 + 8 + 1 + 7 + 8)\n",
         sizeof(struct Worst));

  printf("\noffsets in Mixed:\n");
  printf("  flag  %zu\n", offsetof(struct Mixed, flag));
  printf("  value %zu\n", offsetof(struct Mixed, value));
  printf("  tag   %zu\n", offsetof(struct Mixed, tag));
  printf("  id    %zu\n", offsetof(struct Mixed, id));

  size_t data = sizeof(char) + sizeof(double) + sizeof(char) + sizeof(int);
  printf("\nByAlignment: %zu bytes of data, %zu bytes of padding\n",
         sizeof(struct ByAlignment),
         sizeof(struct ByAlignment) - (sizeof(double) + sizeof(int) + 2));
  printf("Mixed:       %zu bytes of data, %zu bytes of padding\n", data,
         sizeof(struct Mixed) - data);

  /* The rule that produces these numbers: each member starts at an offset that
   * is a multiple of its own alignment, and the struct's size is a multiple of
   * its largest alignment, so an array of them stays aligned. */
  printf("\nlargest alignment in Mixed = %zu\n", alignof(double));
  return 0;
}
