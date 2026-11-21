#ifndef CRAFT_ASSERT_H
#define CRAFT_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

/*
 * CRAFT_ASSERT reports the expression, the file and the line, then aborts.
 * It is used instead of <assert.h> because the message must survive being run
 * from a check script with output captured to a log file.
 */
#define CRAFT_ASSERT(cond)                                                     \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__,     \
              #cond);                                                          \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define CRAFT_ASSERT_MSG(cond, msg)                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "%s:%d: assertion failed: %s (%s)\n", __FILE__,          \
              __LINE__, #cond, (msg));                                         \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#endif /* CRAFT_ASSERT_H */
