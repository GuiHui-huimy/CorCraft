/* LAB 01-02  fasta/fasta_cpp.cpp
 * Drills: the same parser with std::optional at the boundary and owning strings
 *         instead of char pointers, so that the free() obligations disappear.
 * Build:  cmake --build build --target bio_01_fasta_cpp
 */
#include "craft_assert.h"

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Sequence {
  std::string name;
  std::vector<char> residues;
};

static bool is_residue(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '*';
}

/* parse() takes views: it borrows the caller's buffer, and it returns
 * nullopt rather than an exception because bad input is expected input. */
static std::optional<Sequence> parse(std::string_view header,
                                    std::string_view body) {
  if (header.empty() || header.front() != '>') {
    return std::nullopt;
  }
  Sequence s;
  s.name = std::string(header.substr(1));
  s.residues.reserve(body.size());
  for (char c : body) {
    if (c == '\n' || c == '\r' || c == ' ') {
      continue;
    }
    if (!is_residue(c)) {
      return std::nullopt;
    }
    s.residues.push_back(c);
  }
  if (s.residues.empty()) {
    return std::nullopt;
  }
  return s;
}

static double gc_fraction(const Sequence &s) {
  std::size_t gc = 0;
  std::size_t residues = 0;
  for (char c : s.residues) {
    if (c == 'G' || c == 'C' || c == 'g' || c == 'c') {
      ++gc;
    }
    if (is_residue(c)) {
      ++residues;
    }
  }
  if (residues == 0) {
    return 0.0;
  }
  return static_cast<double>(gc) / static_cast<double>(residues);
}

int main() {
  const std::string header = ">sp|P0CG48|UBC_HUMAN Ubiquitin";
  const std::string body =
      "MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQR\r\n"
      "LIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG\r\n";

  auto seq = parse(header, body);
  CRAFT_ASSERT(seq.has_value());
  CRAFT_ASSERT(seq->residues.size() == 76);
  std::cout << seq->name << " length " << seq->residues.size() << " GC "
            << gc_fraction(*seq) << '\n';

  CRAFT_ASSERT(!parse("no header", body).has_value());
  CRAFT_ASSERT(!parse(header, "MQIF1KTL").has_value());
  CRAFT_ASSERT(!parse(header, "\r\n\r\n").has_value());

  /* Lowercase is legal and normalises to the same count. */
  auto lower = parse(header, "mqifvktltgkti");
  CRAFT_ASSERT(lower.has_value());
  CRAFT_ASSERT(lower->residues.size() == 13);

  /* Two records are two calls: the header and body are split by the caller, or
   * by the file reader that produces the pair. */
  auto second = parse(">second", "GGCC");
  CRAFT_ASSERT(second.has_value());
  CRAFT_ASSERT(second->residues.size() == 4);
  std::cout << "second record " << second->name << " length "
            << second->residues.size() << '\n';
  return 0;
}
