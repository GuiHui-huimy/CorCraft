/* LAB 13-01  capstone/sequence_toolkit.cpp
 * Drills: the whole C++ half in one program: an owning Sequence, a Composition
 *         over it, a Fraction with an invariant, and a report written to a
 *         stream so that the output can be compared.
 * Build:  cmake --build build --target cpp_13_capstone
 *
 * The parser reads from a string, not a file, so the lab needs no data
 * directory. docs/bio/01-fasta is the file-based version.
 */
#include "craft_assert.h"

#include <array>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/* An exact fraction with a maintained invariant: reduced, positive denominator,
 * non-zero denominator. Every operation goes through the constructor. */
class Fraction {
public:
  Fraction(long num, long den) : num_(num), den_(den) {
    if (den == 0) {
      throw std::invalid_argument("zero denominator");
    }
    long g = std::gcd(num, den);
    num_ /= g;
    den_ /= g;
    if (den_ < 0) {
      num_ = -num_;
      den_ = -den_;
    }
  }
  long num() const { return num_; }
  long den() const { return den_; }
  friend bool operator==(const Fraction &a, const Fraction &b) {
    return a.num_ == b.num_ && a.den_ == b.den_;
  }
  friend std::ostream &operator<<(std::ostream &out, const Fraction &f) {
    return out << f.num_ << '/' << f.den_;
  }

private:
  long num_;
  long den_;
};

/* Owns its name and residues. Copyable and movable by the rule of zero, because
 * every member is an owning standard type. */
class Sequence {
public:
  Sequence() = default;
  Sequence(std::string name, std::vector<char> residues)
      : name_(std::move(name)), residues_(std::move(residues)) {}

  static std::optional<Sequence> parse(std::string_view header,
                                       std::string_view body) {
    if (header.empty() || header.front() != '>') {
      return std::nullopt;
    }
    Sequence s;
    s.name_ = std::string(header.substr(1));
    s.residues_.reserve(body.size());
    for (char c : body) {
      if (c == '\n' || c == '\r' || c == ' ') {
        continue;
      }
      if (!is_residue(c)) {
        return std::nullopt;
      }
      s.residues_.push_back(c);
    }
    if (s.residues_.empty()) {
      return std::nullopt; // an empty record divides by zero downstream
    }
    return s;
  }

  std::size_t size() const { return residues_.size(); }
  char operator[](std::size_t i) const { return residues_[i]; }
  const std::string &name() const { return name_; }

private:
  static bool is_residue(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '*';
  }
  std::string name_;
  std::vector<char> residues_;
};

class Composition {
public:
  explicit Composition(const Sequence &s) : counts_{} {
    for (std::size_t i = 0; i < s.size(); ++i) {
      char c = s[i];
      if (c >= 'A' && c <= 'Z') {
        ++counts_[static_cast<std::size_t>(c - 'A')];
      } else if (c >= 'a' && c <= 'z') {
        ++counts_[static_cast<std::size_t>(c - 'a')];
      }
    }
  }

  Fraction gc_fraction() const {
    long gc = 0;
    long residues = 0;
    for (std::size_t i = 0; i < 26; ++i) {
      char letter = static_cast<char>('A' + static_cast<int>(i));
      if (letter == 'G' || letter == 'C') {
        gc += static_cast<long>(counts_[i]);
      }
      if (is_residue_letter(letter)) {
        residues += static_cast<long>(counts_[i]);
      }
    }
    if (residues == 0) {
      return Fraction(0, 1);
    }
    return Fraction(gc, residues);
  }

  std::size_t count(char c) const {
    return counts_[static_cast<std::size_t>(c - 'A')];
  }

private:
  static bool is_residue_letter(char c) {
    static const char *letters = "ACDEFGHIKLMNPQRSTVWY";
    for (const char *p = letters; *p != '\0'; ++p) {
      if (*p == c) {
        return true;
      }
    }
    return false;
  }
  std::array<std::size_t, 26> counts_;
};

/* Writes to a stream it is given: that is what makes the output testable. */
static void write_report(std::ostream &out, const Sequence &s,
                         const Composition &c) {
  out << s.name() << '\t' << s.size() << '\t' << c.gc_fraction() << '\n';
}

int main() {
  /* The invariant of Fraction holds for every construction path. */
  CRAFT_ASSERT(Fraction(6, 8) == Fraction(3, 4));
  CRAFT_ASSERT(Fraction(1, -2) == Fraction(-1, 2));
  CRAFT_ASSERT(Fraction(0, 5) == Fraction(0, 1));

  bool threw = false;
  try {
    Fraction bad(1, 0);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  CRAFT_ASSERT(threw);

  /* Ubiquitin, P0CG48, 76 residues. */
  const std::string header = ">sp|P0CG48|UBC_HUMAN Ubiquitin";
  const std::string body =
      "MQIFVKTLTGKTITLEVEPSDTIENVKAKIQDKEGIPPDQQR\n"
      "LIFAGKQLEDGRTLSDYNIQKESTLHLVLRLRGG\n";

  auto seq = Sequence::parse(header, body);
  CRAFT_ASSERT(seq.has_value());
  CRAFT_ASSERT(seq->size() == 76);
  CRAFT_ASSERT(seq->name() == "sp|P0CG48|UBC_HUMAN Ubiquitin");

  Composition comp(*seq);
  CRAFT_ASSERT(comp.gc_fraction().den() > 0);

  std::ostringstream out;
  write_report(out, *seq, comp);
  std::cout << out.str();

  /* The report is checked on its leading fields, not byte for byte: the header
   * is the part a reader is expected to edit. A reduced fraction in the next
   * column keeps that prefix identical on every platform. */
  const std::string expected = "sp|P0CG48|UBC_HUMAN Ubiquitin\t76\t";
  CRAFT_ASSERT(out.str().rfind(expected, 0) == 0);

  /* The rejections: a header without '>', a residue that is not a letter, and
   * an empty record. */
  CRAFT_ASSERT(!Sequence::parse("sp|P0CG48", body).has_value());
  CRAFT_ASSERT(!Sequence::parse(header, "MQIF1KTL").has_value());
  CRAFT_ASSERT(!Sequence::parse(header, "\n\r\n").has_value());

  /* Lowercase is accepted and counts the same. */
  auto lower = Sequence::parse(header, "mqifvktl");
  CRAFT_ASSERT(lower.has_value());
  CRAFT_ASSERT(lower->size() == 8);

  std::cout << "capstone: parse, compose, report, and three rejections\n";
  return 0;
}
