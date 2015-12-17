#include "language_vector.hpp"
#include <memory>
#include <iostream>

int main() {
  const size_t order = 3;
  const size_t n = 10000;

  std::unique_ptr<language_vector::vector> en(
    language_vector::build("this is an impossibly small amount of text, written in English",
                           order, n));
  std::unique_ptr<language_vector::vector> en_more(
    language_vector::build("another document, also written in the Queen's language",
                           order, n));
  language_vector::merge(*en, *en_more);

  std::unique_ptr<language_vector::vector> fr(
    language_vector::build("c'est le premiere heure depuis minuit, non?",
                           order, n));

  std::unique_ptr<language_vector::vector> test(
    language_vector::build("c'est une portion de text Fraincais",
                           order, n));

  // Print out some examples

  std::cout << "en-en: " << language_vector::score(*en, *en) << std::endl;
  std::cout << "en-fr: " << language_vector::score(*en, *fr) << std::endl;
  std::cout << "fr-fr: " << language_vector::score(*fr, *fr) << std::endl;

  std::cout << "Test text en: " << language_vector::score(*en, *test) << std::endl;
  std::cout << "Test text fr: " << language_vector::score(*fr, *test) << std::endl;

  return 0;
}
