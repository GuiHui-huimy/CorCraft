/* LAB 05-02  polymorphism/02_slicing.cpp
 * Drills: passing a polymorphic object by value slices it; the three places
 *         slicing happens and the three fixes.
 * Build:  cmake --build build --target cpp_05_slice
 */
#include "craft_assert.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Animal {
public:
  virtual ~Animal() = default;
  virtual std::string sound() const { return "..."; }
  virtual std::string kind() const { return "Animal"; }
};

class Dog : public Animal {
public:
  std::string sound() const override { return "woof"; }
  std::string kind() const override { return "Dog"; }
};

/* By value: the derived part is cut off at the door. The parameter is an
 * Animal, and only Animal's members were copied. */
static std::string sound_by_value(Animal a) { return a.sound(); }

/* By reference: no copy, dispatch reaches Dog. */
static std::string sound_by_ref(const Animal &a) { return a.sound(); }

int main() {
  Dog d;
  CRAFT_ASSERT(sound_by_ref(d) == "woof");
  CRAFT_ASSERT(sound_by_value(d) == "..."); // sliced, and the call is virtual
                                            // but on a base object

  std::cout << "by value: " << sound_by_value(d) << " (sliced)\n";
  std::cout << "by reference: " << sound_by_ref(d) << '\n';

  /* Second place: a container of values. vector<Animal> stores Animal
   * subobjects, so pushing a Dog stores an Animal. */
  std::vector<Animal> animals;
  animals.push_back(d);
  CRAFT_ASSERT(animals[0].kind() == "Animal");
  std::cout << "vector<Animal> element kind: " << animals[0].kind()
            << " (sliced)\n";

  /* Third place: assignment to a base object. */
  Animal base = d;
  CRAFT_ASSERT(base.kind() == "Animal");

  /* The fixes: references for parameters, pointers or unique_ptr for
   * containers, and a virtual clone() when a polymorphic copy is genuinely
   * wanted. */
  std::vector<std::unique_ptr<Animal>> owned;
  owned.push_back(std::make_unique<Dog>());
  CRAFT_ASSERT(owned[0]->kind() == "Dog");
  std::cout << "vector<unique_ptr<Animal>> element kind: " << owned[0]->kind()
            << '\n';

  std::cout << "sizeof(Animal)=" << sizeof(Animal)
            << " sizeof(Dog)=" << sizeof(Dog)
            << " (no data members, so equal)\n";
  return 0;
}
