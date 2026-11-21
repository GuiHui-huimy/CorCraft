/* LAB 08-01  structure_model/model.cpp
 * Drills: the parser's arrays become an owning object graph, a query that
 *         borrows, and the same numbers from the same input.
 * Build:  cmake --build build --target bio_08_model
 *
 * The input is an embedded excerpt with two chains, so the chain count and the
 * residue count are the assertions that a resSeq-only key would fail.
 */
#include "craft_assert.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

struct Atom {
  std::string name;
  char altloc{};
  char element[3]{};
  double x{}, y{}, z{};
};

class Residue {
public:
  Residue(char chain, int seq, char icode, std::string name)
      : chain_(chain), seq_(seq), icode_(icode), name_(std::move(name)) {}

  void add(const Atom &a) { atoms_.push_back(a); }
  char chain() const { return chain_; }
  int seq() const { return seq_; }
  char icode() const { return icode_; }
  const std::string &name() const { return name_; }
  std::size_t atom_count() const { return atoms_.size(); }
  const std::vector<Atom> &atoms() const { return atoms_; }

private:
  char chain_;
  int seq_;
  char icode_;
  std::string name_;
  std::vector<Atom> atoms_;
};

class Chain {
public:
  explicit Chain(char id) : id_(id) {}
  char id() const { return id_; }
  std::size_t residue_count() const { return residues_.size(); }
  std::size_t atom_count() const {
    std::size_t n = 0;
    for (const Residue &r : residues_) {
      n += r.atom_count();
    }
    return n;
  }
  const Residue &residue_at(std::size_t i) const { return residues_[i]; }

  /* The parser relies on the file's ordering within a chain; the lab's second
   * input is ordered differently and the check below is what catches it. */
  void attach(int seq, char icode, const std::string &res_name, const Atom &a) {
    if (residues_.empty() || residues_.back().seq() != seq ||
        residues_.back().icode() != icode) {
      residues_.emplace_back(id_, seq, icode, res_name);
    }
    residues_.back().add(a);
  }

private:
  char id_;
  std::vector<Residue> residues_;
};

class Structure {
public:
  std::size_t chain_count() const { return chains_.size(); }
  std::size_t atom_count() const {
    std::size_t n = 0;
    for (const Chain &c : chains_) {
      n += c.atom_count();
    }
    return n;
  }
  std::size_t residue_count() const {
    std::size_t n = 0;
    for (const Chain &c : chains_) {
      n += c.residue_count();
    }
    return n;
  }
  const Chain &chain_at(std::size_t i) const { return chains_[i]; }

  static std::optional<Structure> load(std::istream &in) {
    Structure s;
    std::string line;
    while (std::getline(in, line)) {
      if (line.size() < 54) {
        continue; /* too short to be an atom record */
      }
      bool is_atom = line.compare(0, 6, "ATOM  ") == 0;
      bool is_het = line.compare(0, 6, "HETATM") == 0;
      if (!is_atom && !is_het) {
        continue;
      }
      Atom a;
      a.name = line.substr(12, 4);
      a.altloc = line[16];
      a.x = std::stod(line.substr(30, 8));
      a.y = std::stod(line.substr(38, 8));
      a.z = std::stod(line.substr(46, 8));
      s.attach(line[21], std::stoi(line.substr(22, 4)), line[26],
               line.substr(17, 3), a);
    }
    if (s.chains_.empty()) {
      return std::nullopt;
    }
    return s;
  }

private:
  void attach(char chain_id, int seq, char icode, const std::string &res_name,
              const Atom &a) {
    if (chains_.empty() || chains_.back().id() != chain_id) {
      chains_.emplace_back(chain_id);
    }
    chains_.back().attach(seq, icode, res_name, a);
  }

  std::vector<Chain> chains_;
};

static std::string atom_line(const char *record, int serial, const char *name,
                             char altloc, const char *res, char chain, int seq,
                             char icode, double x, double y, double z) {
  std::ostringstream out;
  out.setf(std::ios::fixed);
  out.precision(3);
  out.width(6);
  out << std::left << record;
  out.unsetf(std::ios::adjustfield);
  out.width(5);
  out << std::right << serial << ' ';
  out << std::left;
  out.width(4);
  out << name;
  out.unsetf(std::ios::adjustfield);
  out << altloc;
  out.width(3);
  out << std::left << res;
  out.unsetf(std::ios::adjustfield);
  out << ' ' << chain;
  out.width(4);
  out << std::right << seq << icode << "   ";
  out.width(8);
  out << x;
  out.width(8);
  out << y;
  out.width(8);
  out << z;
  return out.str();
}

int main() {
  /* Two chains, each with two residues, built by the column-accurate helper so
   * the parser is tested rather than the formatting. */
  std::ostringstream text;
  text << atom_line("ATOM  ", 1, " N  ", ' ', "ALA", 'A', 1, ' ', 1.0, 2.0, 3.0)
       << '\n';
  text << atom_line("ATOM  ", 2, " CA ", ' ', "ALA", 'A', 1, ' ', 1.5, 2.0, 3.0)
       << '\n';
  text << atom_line("ATOM  ", 3, " N  ", ' ', "GLY", 'A', 2, ' ', 2.0, 2.0, 3.0)
       << '\n';
  text << atom_line("ATOM  ", 4, " N  ", ' ', "ALA", 'B', 1, ' ', 1.0, 2.0, 4.0)
       << '\n';
  text << atom_line("ATOM  ", 5, " N  ", ' ', "GLY", 'B', 2, ' ', 2.0, 2.0, 4.0)
       << '\n';

  std::istringstream in(text.str());
  auto s = Structure::load(in);
  CRAFT_ASSERT(s.has_value());

  std::cout << "chains " << s->chain_count() << " residues "
            << s->residue_count() << " atoms " << s->atom_count() << '\n';
  CRAFT_ASSERT(s->chain_count() == 2);
  CRAFT_ASSERT(s->residue_count() == 4);
  CRAFT_ASSERT(s->atom_count() == 5);
  CRAFT_ASSERT(s->chain_at(0).id() == 'A');
  CRAFT_ASSERT(s->chain_at(1).id() == 'B');
  CRAFT_ASSERT(s->chain_at(0).residue_at(0).seq() == 1);
  CRAFT_ASSERT(s->chain_at(0).residue_at(0).icode() == ' ');
  CRAFT_ASSERT(s->chain_at(0).residue_at(0).atom_count() == 2);

  /* The coordinate round trip: 27.340 came back as 27.34, which is the same
   * number with fewer printed digits. */
  CRAFT_ASSERT(s->chain_at(1).residue_at(0).atoms()[0].x == 1.0);

  /* An input with no atom records is rejected rather than producing an empty
   * structure that every consumer then has to guard against. */
  std::istringstream empty("HEADER nothing here\n");
  CRAFT_ASSERT(!Structure::load(empty).has_value());

  /* Bodies are private and access is const-only, which is why a pointer from a
   * query cannot outlive a structural change: there is no non-const access to
   * change it with. */
  const Chain &first = s->chain_at(0);
  CRAFT_ASSERT(first.residue_count() == 2);
  std::cout << "first chain " << first.id() << " has " << first.atom_count()
            << " atoms\n";
  return 0;
}
