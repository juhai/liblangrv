#include "language_vector.hpp"
#include <memory>
#include <iostream>
#include <catch.hpp>

namespace {

  std::unique_ptr<language_vector::builder> make_builder(size_t order = 3) {
    return std::unique_ptr<language_vector::builder>{language_vector::make_builder(order, 10000, 42)};
  }

  std::unique_ptr<language_vector::vector> build(const std::string& text, size_t order = 3) {
    auto builder = make_builder(order);
    return std::unique_ptr<language_vector::vector>{(*builder)(text)};
  }

} // namespace (anonymous)

using Catch::Detail::Approx;

TEST_CASE("Language vectors can be built and scored", "") {
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

TEST_CASE("Language vectors can be merged", "") {
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

TEST_CASE("Language vectors can be serialized/deserialized", "[io]") {
  auto builder = make_builder();
  auto round_trip = [&builder](const language_vector::vector& original) {
    std::stringstream stream;
    builder->save(original, stream);
    return std::unique_ptr<language_vector::vector>{builder->load(stream)};
  };

  auto x = (*builder)("This is some text");
  auto y = (*builder)("C'est autres text");

  auto x_reload = round_trip(*x);
  auto y_reload = round_trip(*y);

  REQUIRE(language_vector::score(*x, *x_reload) == Approx(1));
  REQUIRE(language_vector::score(*y, *y_reload) == Approx(1));
  REQUIRE(language_vector::score(*x_reload, *y_reload) < 0.99f);
}

TEST_CASE("Example of larger language vectors", "") {
  auto en = build("this is an impossibly small amount of text, written in English");
  auto en_more = build("another document, also written in the Queen's language");
  language_vector::merge(*en, *en_more);
  auto fr = build("c'est le premiere heure depuis minuit, non?");

  REQUIRE(language_vector::score(*en, *en) == Approx(1));
  REQUIRE(language_vector::score(*fr, *fr) == Approx(1));
  REQUIRE(language_vector::score(*en, *fr) == Approx(language_vector::score(*fr, *en)));
  REQUIRE(language_vector::score(*en, *fr) < 0.99f);
}
