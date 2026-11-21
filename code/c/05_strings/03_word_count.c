/* LAB 05-03  strings/03_word_count.c
 * Drills: counting words without splitting the string, and the state machine
 *         that distinguishes inside a word from outside one.
 * Build:  cmake --build build --target c_05_words
 */
#include "craft_assert.h"

#include <stdio.h>

static int is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static size_t word_count(const char *s) {
  size_t words = 0;
  int in_word = 0;
  for (const char *p = s; *p != '\0'; ++p) {
    if (is_space(*p)) {
      in_word = 0;
    } else if (!in_word) {
      in_word = 1;
      ++words;
    }
  }
  return words;
}

int main(void) {
  CRAFT_ASSERT(word_count("") == 0);
  CRAFT_ASSERT(word_count("   ") == 0);
  CRAFT_ASSERT(word_count("one") == 1);
  CRAFT_ASSERT(word_count("one two three") == 3);
  CRAFT_ASSERT(word_count("  leading and trailing  ") == 3);
  CRAFT_ASSERT(word_count("tab\tseparated\nlines") == 3);
  CRAFT_ASSERT(word_count("one  two") == 2); /* two spaces are one separator */

  printf("words in \"one two three\" = %zu\n",
         word_count("one two three"));

  /* The defect that a splitter hides: strtok modifies the input and cannot be
   * nested. The state machine reads the string and changes nothing, which is
   * why it works on a const char *. */
  const char *line = "read only input";
  printf("count on a const string = %zu\n", word_count(line));
  return 0;
}
