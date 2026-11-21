/* LAB 02-01  classes/01_interface_split.cpp
 * Drills: the class as an invariant, const member functions, and the header
 *         that declares while the implementation stays here.
 * Build:  cmake --build build --target cpp_02_classes
 *
 * The class is declared and defined in one file for the lab. The split into
 * .h and .cpp is the same shape with the definition moved out.
 */
#include "craft_assert.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class GradeBook {
public:
  explicit GradeBook(std::string course) : course_(std::move(course)) {}

  /* Not const: it changes the invariant, so it must be callable only on a
   * non-const object. A const method here would be a lie the compiler enforces
   * against the members. */
  void add(double score) {
    if (score < 0.0 || score > 100.0) {
      throw std::out_of_range("score must be in [0, 100]");
    }
    scores_.push_back(score);
  }

  /* const: promises not to change the object, so it is callable on a const
   * reference and on a temporary. */
  double mean() const {
    if (scores_.empty()) {
      return 0.0;
    }
    double sum = 0.0;
    for (double s : scores_) {
      sum += s;
    }
    return sum / static_cast<double>(scores_.size());
  }

  double max() const {
    return scores_.empty() ? 0.0 : *std::max_element(scores_.begin(), scores_.end());
  }

  std::size_t size() const { return scores_.size(); }
  const std::string &course() const { return course_; }

private:
  std::string course_;
  std::vector<double> scores_; // the invariant: every element is in [0, 100]
};

/* The interface is what a caller sees; the two data members are the reason the
 * invariant can be maintained at all. */
int main() {
  GradeBook book("C/C++ Craft");
  book.add(88.0);
  book.add(92.5);
  book.add(75.0);

  CRAFT_ASSERT(book.size() == 3);
  CRAFT_ASSERT(book.max() == 92.5);
  std::cout << "mean " << book.mean() << " max " << book.max() << '\n';

  bool rejected = false;
  try {
    book.add(120.0);
  } catch (const std::out_of_range &) {
    rejected = true;
  }
  CRAFT_ASSERT(rejected);
  CRAFT_ASSERT(book.size() == 3); // the rejected value did not enter

  /* A const reference can call only the const members, which is the compiler
   * enforcing the promise rather than the comment claiming it. */
  const GradeBook &view = book;
  CRAFT_ASSERT(view.size() == 3);
  std::cout << "course " << view.course() << '\n';

  return 0;
}
