#include "language_vector.hpp"
#include <memory>
#include <iostream>
#include <catch.hpp>

std::unique_ptr<language_vector::vector> build(const std::string& text, size_t order = 3) {
  // 10000
  return std::unique_ptr<language_vector::vector>(language_vector::build(text, order, 10000, 42));
}

using Catch::Detail::Approx;

TEST_CASE("Language vectors can be built and scored", "[]") {
  auto easy = build("abc 123");
  // full overlap
  REQUIRE(language_vector::score(*easy, *easy) == Approx(1));
  REQUIRE(language_vector::score(*easy, *build("abc 123")) == Approx(1));

  // partial overlap
  REQUIRE(language_vector::score(*easy, *build("abc")) < 0.99f);
  REQUIRE(language_vector::score(*easy, *build("123 abc")) < 0.99f);

  // there is no overlap here (just random coincidence of ngram vectors)
  REQUIRE(language_vector::score(*build("vwxyz"), *build("abcde")) < 0.01f);
}

TEST_CASE("Language vectors obey ngram invariance", "[ngram]") {
  // bigrams should be agnostic to the following, as the ngrams are both
  // .a a. .b b.
  REQUIRE(language_vector::score(*build(".a.b.", 2), *build(".b.a.", 2)) == Approx(1));
  REQUIRE(language_vector::score(*build(".a.b."), *build(".b.a.")) < 0.99f);

  // trigrams should be agnostic to the following, as the ngrams are both
  // ..a .a. a.. ..b .b. b..
  REQUIRE(language_vector::score(*build("..a..b.."), *build("..b..a..")) == Approx(1));
}

TEST_CASE("Language vectors can be merged", "[]") {
  // merge two identical documents in different orders - should be the same
  auto a = build("abcdef");
  language_vector::merge(*a, *build("12345"));
  auto b = build("12345");
  language_vector::merge(*b, *build("abcdef"));
  REQUIRE(language_vector::score(*a, *b) == Approx(1));

  // compare a 'doubled up' merged language vector to a single vector
  auto once = build("abc def");
  auto twice = build("abc def");
  language_vector::merge(*twice, *build("abc def"));
  REQUIRE(language_vector::score(*once, *twice) == Approx(1));

  // merging should move a language vector away from the original
  auto x = build("abcdef");
  language_vector::merge(*x, *build("fedcba"));
  REQUIRE(language_vector::score(*x, *build("abcdef")) < 0.99f);
}

TEST_CASE("Example", "[example]") {
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
}
