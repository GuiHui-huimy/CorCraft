/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 11-01  modern/01_auto_and_structured.cpp
 * Drills: auto that avoids a narrowing, structured bindings that name both
 *         halves of a result, if-with-initialiser, string_view as a borrow,
 *         optional as an explicit absence.
 * Build:  cmake --build build --target cpp_11_modern
 */
#include "craft_assert.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

static std::optional<std::size_t> find_first(std::string_view hay,
                                             std::string_view needle) {
  std::size_t at = hay.find(needle);
  if (at == std::string_view::npos) {
    return std::nullopt; // absence is in the return type, not in a sentinel
  }
  return at;
}

/* A string_view parameter accepts a string literal, a std::string, and a
 * substring without allocating any of them. */
static std::size_t gc_count(std::string_view seq) {
  std::size_t n = 0;
  for (char c : seq) {
    if (c == 'G' || c == 'C' || c == 'g' || c == 'c') {
      ++n;
    }
  }
  return n;
}

int main() {
  std::vector<int> v{1, 2, 3, 4};

  /* auto takes the type of the expression: std::size_t, which is what size()
   * returns. `int n = v.size()` would narrow on a 64-bit platform. */
  auto n = v.size();
  CRAFT_ASSERT(n == 4);
  static_assert(sizeof(n) == sizeof(std::size_t), "auto preserved the width");

  /* The counter-example: auto on an integer division keeps the integer type,
   * and the value is 0, not 0.5. Writing double would have caught it. */
  auto ratio = 1 / 2;
  CRAFT_ASSERT(ratio == 0);
  double real_ratio = 1 / 2.0;
  CRAFT_ASSERT(real_ratio == 0.5);
  std::cout << "auto ratio=" << ratio << " double ratio=" << real_ratio << '\n';

  /* Structured bindings on a map: both the key and the value are named. */
  std::map<std::string, int> counts{{"G", 3}, {"C", 2}, {"A", 5}};
  for (const auto &[word, count] : counts) {
    CRAFT_ASSERT(!word.empty());
    CRAFT_ASSERT(count > 0);
  }

  /* try_emplace returns a pair; the binding makes both halves usable without a
   * .first / .second. */
  auto [it, inserted] = counts.try_emplace("T", 0);
  CRAFT_ASSERT(inserted);
  auto [it2, inserted2] = counts.try_emplace("T", 9);
  CRAFT_ASSERT(!inserted2); // the existing entry is not overwritten
  CRAFT_ASSERT(it2->second == 0);
  (void)it;

  /* if with an initialiser: the iterator is scoped to the branch and cannot
   * leak into the rest of the function. */
  if (auto found = counts.find("G"); found != counts.end()) {
    CRAFT_ASSERT(found->second == 3);
    std::cout << "G counted " << found->second << " times\n";
  }

  /* string_view from a string literal: the literal outlives the call. */
  std::string seq = "ACGTACGT";
  CRAFT_ASSERT(gc_count(seq) == 4);
  CRAFT_ASSERT(gc_count("GGCC") == 4);

  CRAFT_ASSERT(find_first(seq, "TAC") == 3);
  CRAFT_ASSERT(!find_first(seq, "NNN").has_value());

  /* The dangling case is described, not run: a view over a temporary dies at
   * the end of the expression that created it. */
  std::string_view view = seq; // safe: seq outlives view
  CRAFT_ASSERT(view.size() == 8);
  std::cout << "sequence " << view << " has " << gc_count(view) << " GC\n";

  std::optional<int> absent;
  CRAFT_ASSERT(!absent.has_value());
  absent = 7;
  CRAFT_ASSERT(absent.value() == 7);
  return 0;
}
