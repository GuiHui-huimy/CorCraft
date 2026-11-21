/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 11-02  callbacks/02_dispatch_table.c
 * Drills: a table of function pointers as a command dispatcher, and the switch
 *         it replaces.
 * Build:  cmake --build build --target c_11_dispatch
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*command_fn)(int argc, char **argv);

static int cmd_add(int argc, char **argv) {
  int sum = 0;
  for (int i = 1; i < argc; ++i) {
    sum += atoi(argv[i]);
  }
  printf("add: %d\n", sum);
  return 0;
}

static int cmd_len(int argc, char **argv) {
  int total = 0;
  for (int i = 1; i < argc; ++i) {
    total += (int)strlen(argv[i]);
  }
  printf("len: %d\n", total);
  return 0;
}

static int cmd_echo(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    printf("echo: %s\n", argv[i]);
  }
  return 0;
}

typedef struct {
  const char *name;
  command_fn fn;
} entry;

/* NULL-terminated, so the loop has one stopping condition rather than a count
 * that must be kept in sync with the table. */
static const entry table[] = {
    {"add", cmd_add}, {"len", cmd_len}, {"echo", cmd_echo}, {NULL, NULL}};

static const entry *lookup(const char *name) {
  for (const entry *e = table; e->name != NULL; ++e) {
    if (strcmp(e->name, name) == 0) {
      return e;
    }
  }
  return NULL;
}

static int dispatch(const char *name, int argc, char **argv) {
  const entry *e = lookup(name);
  if (e == NULL) {
    fprintf(stderr, "unknown command: %s\n", name);
    return 1;
  }
  return e->fn(argc, argv);
}

int main(void) {
  /* The table is walked in a test as well as used in dispatch, which is why
   * lookup and dispatch are separate: one answers "is it there", the other
   * does the work. */
  CRAFT_ASSERT(lookup("add") != NULL);
  CRAFT_ASSERT(lookup("len") != NULL);
  CRAFT_ASSERT(lookup("nope") == NULL);

  const char *add_args[] = {"add", "2", "3", "4"};
  CRAFT_ASSERT(dispatch("add", 4, (char **)add_args) == 0);

  const char *len_args[] = {"len", "abcd", "ef"};
  CRAFT_ASSERT(dispatch("len", 3, (char **)len_args) == 0);

  CRAFT_ASSERT(dispatch("nope", 1, NULL) == 1);

  /* The comparison the chapter makes: a table is data, a switch is code.
   * Adding a command to the table is one line and no new branch; adding one to
   * a switch is a new case and a recompile of every caller. */
  printf("commands in table: %zu\n",
         sizeof table / sizeof table[0] - 1);
  return 0;
}
