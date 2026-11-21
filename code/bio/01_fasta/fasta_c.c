/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 01-01  fasta/fasta_c.c
 * Drills: parse FASTA into owned records, reject what the format allows but the
 *         downstream cannot use, and free every level exactly once.
 * Build:  cmake --build build --target bio_01_fasta_c
 *
 * The input is embedded so the lab runs without a data directory. The file
 * version is the same parser plus craft_read_file.
 */
#include "craft_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *name;     /* owned */
  char *residues; /* owned, NUL-terminated */
  size_t length;
} record;

typedef struct {
  record *items;
  size_t count;
  size_t capacity;
} record_list;

static void list_init(record_list *l) {
  l->items = NULL;
  l->count = 0;
  l->capacity = 0;
}

static void list_free(record_list *l) {
  for (size_t i = 0; i < l->count; ++i) {
    free(l->items[i].name);
    free(l->items[i].residues);
  }
  free(l->items);
  list_init(l);
}

static int list_push(record_list *l, record r) {
  if (l->count == l->capacity) {
    size_t next = l->capacity == 0 ? 4 : l->capacity * 2;
    record *grown = (record *)realloc(l->items, next * sizeof *grown);
    if (grown == NULL) {
      return -1;
    }
    l->items = grown;
    l->capacity = next;
  }
  l->items[l->count++] = r;
  return 0;
}

static int is_residue(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '*';
}

/* One pass over the text. The two record types mean the parser must keep state:
 * inside a header, or reading residues. */
static int parse(const char *text, record_list *out) {
  size_t i = 0;
  while (text[i] != '\0') {
    while (text[i] == '\n' || text[i] == '\r') {
      ++i;
    }
    if (text[i] == '\0') {
      break;
    }
    if (text[i] != '>') {
      return -1; /* a residue line before any header */
    }

    size_t header_start = i + 1;
    size_t header_end = header_start;
    while (text[header_end] != '\n' && text[header_end] != '\r' &&
           text[header_end] != '\0') {
      ++header_end;
    }
    i = header_end;

    size_t cap = 64;
    char *residues = (char *)malloc(cap);
    if (residues == NULL) {
      return -1;
    }
    size_t len = 0;

    while (text[i] != '\0' && text[i] != '>') {
      char c = text[i++];
      if (c == '\n' || c == '\r' || c == ' ') {
        continue;
      }
      if (!is_residue(c)) {
        free(residues);
        return -1;
      }
      if (len + 1 >= cap) {
        size_t next = cap * 2;
        char *grown = (char *)realloc(residues, next);
        if (grown == NULL) {
          free(residues);
          return -1;
        }
        residues = grown;
        cap = next;
      }
      residues[len++] = c;
    }
    if (len == 0) {
      free(residues);
      return -1; /* an empty record divides by zero later */
    }
    residues[len] = '\0';

    record r;
    r.length = len;
    r.residues = residues;
    size_t name_len = header_end - header_start;
    r.name = (char *)malloc(name_len + 1);
    if (r.name == NULL) {
      free(residues);
      return -1;
    }
    memcpy(r.name, text + header_start, name_len);
    r.name[name_len] = '\0';

    if (list_push(out, r) != 0) {
      free(r.name);
      free(r.residues);
      return -1;
    }
  }
  return 0;
}

int main(void) {
  /* CRLF endings, a wrapped body, and a lowercase record: all legal. */
  const char *ubiquitin =
      ">sp|P0CG48|UBC_HUMAN Ubiquitin\r\n"
      "MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQR\r\n"
      "LIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG\r\n";

  record_list list;
  list_init(&list);

  int rc = parse(ubiquitin, &list);
  CRAFT_ASSERT(rc == 0);
  CRAFT_ASSERT(list.count == 1);
  CRAFT_ASSERT(list.items[0].length == 76);
  CRAFT_ASSERT(strcmp(list.items[0].name, "sp|P0CG48|UBC_HUMAN Ubiquitin") == 0);
  printf("records %zu, length %zu\n", list.count, list.items[0].length);

  /* The rejections the parser must make: no '>', a non-residue character, and
   * an empty record. */
  record_list bad;
  list_init(&bad);
  CRAFT_ASSERT(parse("MQIFV", &bad) == -1);
  CRAFT_ASSERT(parse(">x\nMQIF1KTL\n", &bad) == -1);
  CRAFT_ASSERT(parse(">x\n\n", &bad) == -1);
  CRAFT_ASSERT(bad.count == 0);
  list_free(&bad);

  /* A second record in the same text: the parser must produce two, not one. */
  record_list two;
  list_init(&two);
  CRAFT_ASSERT(parse(">a\nACGT\n>b\nGGCC\n", &two) == 0);
  CRAFT_ASSERT(two.count == 2);
  CRAFT_ASSERT(two.items[1].length == 4);
  printf("second record: %s, %zu residues\n", two.items[1].name,
         two.items[1].length);
  list_free(&two);

  list_free(&list);
  printf("freed every level\n");
  return 0;
}
