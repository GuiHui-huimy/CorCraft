/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 10-01  algorithms/01_lambda_pipeline.cpp
 * Drills: sort and stable_sort, a strict weak ordering, find_if, transform,
 *         and the erase-remove idiom with a lambda.
 * Build:  cmake --build build --target cpp_10_algorithms
 */
#include "craft_assert.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Record {
  std::string name;
  int score;
  int order; // the input position, used to show that sort is not stable
};

/* A strict weak ordering: a < b, never a <= b. The <= form violates
 * irreflexivity and makes std::sort read out of bounds. */
static bool by_score(const Record &a, const Record &b) {
  return a.score < b.score;
}

int main() {
  std::vector<Record> records{{"alpha", 70, 0}, {"beta", 90, 1},
                              {"gamma", 70, 2}, {"delta", 55, 3}};

  std::vector<Record> unstable = records;
  std::sort(unstable.begin(), unstable.end(), by_score);
  CRAFT_ASSERT(unstable.front().name == "delta");
  CRAFT_ASSERT(unstable.back().name == "beta");

  /* stable_sort keeps the input order among equal scores, which is why a
   * report sorted by score still reads in the original sequence. */
  std::vector<Record> stable = records;
  std::stable_sort(stable.begin(), stable.end(), by_score);
  CRAFT_ASSERT(stable[0].name == "delta");
  CRAFT_ASSERT(stable[1].name == "alpha"); // 70 came before gamma
  CRAFT_ASSERT(stable[2].name == "gamma");

  std::cout << "stable by score: ";
  for (const Record &r : stable) {
    std::cout << r.name << '(' << r.score << ") ";
  }
  std::cout << '\n';

  /* find_if returns an iterator; the check against end() is the caller's. */
  auto it = std::find_if(stable.begin(), stable.end(),
                         [](const Record &r) { return r.score >= 90; });
  CRAFT_ASSERT(it != stable.end());
  CRAFT_ASSERT(it->name == "beta");
  std::cout << "first >= 90: " << it->name << '\n';

  /* transform writes through an output iterator and does not resize, so the
   * destination already needs the room. */
  std::vector<int> scores(stable.size());
  std::transform(stable.begin(), stable.end(), scores.begin(),
                 [](const Record &r) { return r.score; });
  CRAFT_ASSERT(scores.size() == 4);
  CRAFT_ASSERT(scores.front() == 55);

  /* erase-remove with a lambda: the predicate is the only thing that changes. */
  std::vector<Record> passed = records;
  passed.erase(std::remove_if(passed.begin(), passed.end(),
                              [](const Record &r) { return r.score < 60; }),
               passed.end());
  CRAFT_ASSERT(passed.size() == 3);
  CRAFT_ASSERT(passed.front().name == "alpha");
  std::cout << "after removing < 60: " << passed.size() << " records\n";

  /* Three records score at least 70: alpha, beta and gamma. The count is the
   * check that the capture is by value and not by reference to a dangling
   * local. */
  int threshold = 70;
  auto at_least = [threshold](const Record &r) { return r.score >= threshold; };
  CRAFT_ASSERT(std::count_if(records.begin(), records.end(), at_least) == 3);

  /* The comparator that subtracts returns a difference used as a truth value,
   * so it reports "less than" for any unequal pair and breaks the ordering. */
  std::cout << "a comparator must return a<bool>, never a - b\n";
  return 0;
}
