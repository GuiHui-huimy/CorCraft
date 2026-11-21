/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 07-01  templates/01_template_diag.cpp
 * Drills: the two defects a template hides until it is instantiated, a
 *         comparator whose condition sorts the wrong way, and the deduction
 *         failure that produces the long diagnostic.
 * Build:  cmake --build build --target cpp_07_templates
 */
#include "craft_assert.h"

#include <iostream>
#include <string>
#include <vector>

/* Defect 1: the loop compares arr[0] with itself, so the minimum is always the
 * first element. It compiles for every type with operator<. */
template <typename T> T minimum_buggy(T *arr, std::size_t n) {
  T min = arr[0];
  for (std::size_t i = 0; i < n; ++i) {
    if (arr[0] < min) { // should be arr[i]
      min = arr[0];
    }
  }
  return min;
}

template <typename T> T minimum(T *arr, std::size_t n) {
  T min = arr[0];
  for (std::size_t i = 1; i < n; ++i) {
    if (arr[i] < min) {
      min = arr[i];
    }
  }
  return min;
}

/* Defect 2: the condition swaps when a[i] < a[j], which moves the larger value
 * to the front. The result is sorted, descending. */
template <typename T> void sort_buggy(std::vector<T> &v) {
  for (std::size_t i = 0; i < v.size(); ++i) {
    for (std::size_t j = i; j < v.size(); ++j) {
      if (v[i] < v[j]) {
        T tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
      }
    }
  }
}

template <typename T> void sort_ascending(std::vector<T> &v) {
  for (std::size_t i = 0; i < v.size(); ++i) {
    for (std::size_t j = i + 1; j < v.size(); ++j) {
      if (v[j] < v[i]) {
        T tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
      }
    }
  }
}

/* Explicit deduction failure: two T parameters and two different argument
 * types. The fix is a cast or a common type, and the lab shows both. */
template <typename T> T larger(T a, T b) { return a > b ? a : b; }

int main() {
  int values[] = {9, 4, 7, 1, 8};
  CRAFT_ASSERT(minimum(values, 5) == 1);
  CRAFT_ASSERT(minimum_buggy(values, 5) == 9); // the defect, asserted
  std::cout << "minimum=" << minimum(values, 5)
            << " buggy=" << minimum_buggy(values, 5) << '\n';

  std::vector<int> v{3, 1, 4, 1, 5};
  std::vector<int> w = v;
  sort_ascending(v);
  sort_buggy(w);
  CRAFT_ASSERT(v.front() == 1);
  CRAFT_ASSERT(v.back() == 5);
  CRAFT_ASSERT(w.front() == 5); // descending
  CRAFT_ASSERT(w.back() == 1);

  std::cout << "ascending ";
  for (int x : v) {
    std::cout << x << ' ';
  }
  std::cout << "\ndescending ";
  for (int x : w) {
    std::cout << x << ' ';
  }
  std::cout << '\n';

  CRAFT_ASSERT(larger<double>(3, 4.5) == 4.5); // explicit argument resolves it

  /* Instantiation is lazy: minimum<std::string> is not compiled until this
   * line exists, which is why a defect inside an unused template is invisible. */
  std::string words[] = {"pear", "apple", "fig"};
  CRAFT_ASSERT(minimum(words, 3) == "apple");
  std::cout << "alphabetic min: " << minimum(words, 3) << '\n';
  return 0;
}
