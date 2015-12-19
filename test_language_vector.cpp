#include "language_vector.hpp"
#include <memory>
#include <iostream>

int main() {
  const size_t order = 3;
  const size_t n = 10000;
  const size_t seed = 42;

  auto build = [=](const std::string& text) {
    return std::unique_ptr<language_vector::vector>(language_vector::build(text, order, n, seed));
  };

  auto en = build("this is an impossibly small amount of text, written in English");
  auto en_more = build("another document, also written in the Queen's language");
  language_vector::merge(*en, *en_more);
  auto fr = build("c'est le premiere heure depuis minuit, non?");
  auto test = build("c'est une portion de text Fraincais");

  // Print out some examples

  std::cout << "en-en: " << language_vector::score(*en, *en) << std::endl;
  std::cout << "en-fr: " << language_vector::score(*en, *fr) << std::endl;
  std::cout << "fr-fr: " << language_vector::score(*fr, *fr) << std::endl;

  std::cout << "Test text en: " << language_vector::score(*en, *test) << std::endl;
  std::cout << "Test text fr: " << language_vector::score(*fr, *test) << std::endl;

  return 0;
}
