/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 09-01  containers/01_invalidation.cpp
 * Drills: what a reallocation invalidates, reserve as the fix, and the
 *         erase-remove idiom.
 * Build:  cmake --build build --target cpp_09_containers
 *
 * Default mode keeps every iterator valid. --unsafe keeps a pointer across a
 * push_back that reallocates, for ASan to report.
 */
#include "craft_assert.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv) {
  bool unsafe = (argc > 1 && std::string(argv[1]) == "--unsafe");

  std::vector<int> v{1, 2, 3};
  std::cout << "size " << v.size() << " capacity " << v.capacity()
            << " sizeof(vector) " << sizeof(v) << '\n';

  /* reserve removes the reallocation, and with it the invalidation. */
  v.reserve(100);
  int *data = v.data();
  for (int i = 0; i < 50; ++i) {
    v.push_back(i);
  }
  CRAFT_ASSERT(v.data() == data); // the promise reserve makes
  CRAFT_ASSERT(v.front() == 1);

  if (unsafe) {
    std::vector<int> small{1, 2, 3};
    int *stale = small.data();
    for (int i = 0; i < 100; ++i) {
      small.push_back(i); // reallocates, stale dangles
    }
    std::cout << "reading a stale pointer: " << stale[0] << '\n';
  } else {
    std::vector<int> small{1, 2, 3};
    size_t cap = small.capacity();
    size_t moved = 0;
    for (int i = 0; i < 100; ++i) {
      small.push_back(i);
      if (small.capacity() != cap) {
        ++moved;
        cap = small.capacity();
      }
    }
    std::cout << "growth from 3 to 103 elements: " << moved
              << " reallocations\n";
  }

  /* The erase-remove idiom: remove_if moves the kept elements to the front and
   * returns the new end; erase destroys the tail. */
  std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8};
  numbers.erase(std::remove_if(numbers.begin(), numbers.end(),
                               [](int x) { return x % 2 == 0; }),
                numbers.end());
  CRAFT_ASSERT(numbers.size() == 4);
  CRAFT_ASSERT(numbers[0] == 1 && numbers[3] == 7);
  std::cout << "after erase-remove: ";
  for (int x : numbers) {
    std::cout << x << ' ';
  }
  std::cout << '\n';

  /* Iterating and erasing in the same loop is the defect the idiom replaces:
   * erase returns the next iterator, and a for loop ignores that. */
  std::cout << "erasing in a range-for invalidates the iterator\n";
  return 0;
}
