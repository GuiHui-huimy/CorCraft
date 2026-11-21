/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 01-01  objects_in_memory/01_sizeof_table.c
 * Drills: sizeof of scalar types, sizeof of an expression, array vs pointer.
 * Build:  cmake --build build --target c_01_sizeof
 */
#include <stddef.h>
#include <stdio.h>

int main(void) {
  printf("char            %2zu\n", sizeof(char));
  printf("short           %2zu\n", sizeof(short));
  printf("int             %2zu\n", sizeof(int));
  printf("long            %2zu\n", sizeof(long));
  printf("long long       %2zu\n", sizeof(long long));
  printf("float           %2zu\n", sizeof(float));
  printf("double          %2zu\n", sizeof(double));
  printf("long double     %2zu\n", sizeof(long double));
  printf("pointer         %2zu\n", sizeof(void *));
  printf("size_t          %2zu\n", sizeof(size_t));

  /* The two rows that surprise people: in C a character constant has type int,
   * and sizeof is computed from the type, never from the value. */
  printf("sizeof('a')     %2zu  (C: an int, not a char)\n", sizeof('a'));
  printf("sizeof(1 + 2)   %2zu\n", sizeof(1 + 2));
  printf("sizeof(1.0f)    %2zu\n", sizeof(1.0f));

  int arr[10];
  printf("sizeof(arr)     %2zu  (10 * sizeof(int))\n", sizeof arr);
  printf("sizeof(arr[0])  %2zu\n", sizeof arr[0]);

  /* An expression's type decides its size, so a char added to an int is an int. */
  char c = 'x';
  printf("sizeof(c + 1)   %2zu  (promoted to int)\n", sizeof(c + 1));
  return 0;
}
